#include "renderer.h"
#include "render_scene.h"
#include "render_context.h"
#include "vertex_factory.h"
#include "utils/debug_utils.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::render(RenderContext* ctx)
{
    prepare(ctx);
    render_opaque_pass(ctx);
    copy_to_screen(ctx);
}

void Renderer::prepare(RenderContext* ctx)
{
    // Prepare FrameConstants
    FrameConstants frame_constants;
    frame_constants.view_matrix = g_scene->view[DISPLAY_VIEW].view;
    frame_constants.proj_matrix = g_scene->view[DISPLAY_VIEW].proj;
    GpuBuffer* frame_ub = ctx->create_ub("u_frame", sizeof(FrameConstants));
    frame_ub->write((uint8_t*)&frame_constants, sizeof(FrameConstants));

    // Prepare RTs
    TextureRef* out_color_ref = ctx->get_texture_ref("out_color");
    uint32_t rt_width = out_color_ref->get_desc().width;
    uint32_t rt_height = out_color_ref->get_desc().height;

    RenderContext::CreateStatus create_status;
    EzTextureDesc texture_desc{};
    texture_desc.width = rt_width;
    texture_desc.height = rt_height;
    texture_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    texture_desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    TextureRef* scene_color_ref = ctx->create_texture_ref("scene_color", texture_desc, create_status);
    if (create_status == RenderContext::CreateStatus::Recreated)
    {
        ez_create_texture_view(scene_color_ref->get_texture(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    }

    texture_desc.format = VK_FORMAT_D24_UNORM_S8_UINT;
    texture_desc.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    TextureRef* scene_depth_ref = ctx->create_texture_ref("scene_depth", texture_desc, create_status);
    if (create_status == RenderContext::CreateStatus::Recreated)
    {
        ez_create_texture_view(scene_depth_ref->get_texture(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1);
    }

    VkImageMemoryBarrier2 rt_barriers[2];
    rt_barriers[0] = ez_image_barrier(scene_color_ref->get_texture(), EZ_RESOURCE_STATE_RENDERTARGET);
    rt_barriers[1] = ez_image_barrier(scene_depth_ref->get_texture(), EZ_RESOURCE_STATE_DEPTH_WRITE);
    ez_pipeline_barrier(0, 0, nullptr, 2, rt_barriers);

    // Prepare DrawList
    for (int i = 0; i < DRAW_MAXCOUNT; ++i)
    {
        draw_lists[i].clear();
    }

    VertexFactory* last_vertex_factory = nullptr;
    std::vector<SceneInstanceData> scene_data;
    for (int i = 0; i < g_scene->renderables.size(); ++i)
    {
        Renderable* renderable = g_scene->renderables[i];
        glm::mat4& renderable_transform = renderable->transform;
        glm::vec3 renderable_position = glm::vec3(renderable_transform[3][0], renderable_transform[3][1], renderable_transform[3][2]);

        if (last_vertex_factory != renderable->vertex_factory)
        {
            SceneInstanceData scene_instance;
            scene_instance.transform = renderable_transform;
            scene_data.push_back(scene_instance);

            last_vertex_factory = renderable->vertex_factory;
        }

        DrawCommand* draw_cmd = draw_lists[renderable->draw_type].add_element();
        draw_cmd->scene_index = scene_data.size() - 1;
        draw_cmd->vertex_factory = renderable->vertex_factory;
        draw_cmd->program = renderable->program;
        draw_cmd->distance = glm::distance(g_scene->view[DISPLAY_VIEW].position, renderable_position);
        draw_cmd->sort.sort_key = 0;
    }

    GpuBuffer* scene_ub = ctx->create_ub("u_scene", sizeof(SceneInstanceData) * scene_data.size());
    scene_ub->write((uint8_t*)scene_data.data(), sizeof(SceneInstanceData) * scene_data.size());
}

void Renderer::render_opaque_pass(RenderContext* ctx)
{
    DebugLabel debug_label("Render opaque pass", DebugLabel::WHITE);

    TextureRef* scene_color_ref = ctx->get_texture_ref("scene_color");
    TextureRef* scene_depth_ref = ctx->get_texture_ref("scene_depth");
    uint32_t rt_width = scene_color_ref->get_desc().width;
    uint32_t rt_height = scene_color_ref->get_desc().height;

    ez_reset_pipeline_state();

    EzRenderingInfo rendering_info{};
    EzRenderingAttachmentInfo color_info{};
    color_info.texture = scene_color_ref->get_texture();
    color_info.clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
    rendering_info.colors.push_back(color_info);

    EzRenderingAttachmentInfo depth_info{};
    depth_info.texture = scene_depth_ref->get_texture();
    depth_info.clear_value.depthStencil = {1.0f, 1};
    rendering_info.depth.push_back(depth_info);

    rendering_info.width = rt_width;
    rendering_info.height = rt_height;

    ez_begin_rendering(rendering_info);

    glm::vec4 vp = ctx->viewport_size;
    ez_set_viewport(vp.x, vp.y, vp.z, vp.w);
    ez_set_scissor((int32_t)vp.x, (int32_t)vp.y, (int32_t)vp.z, (int32_t)vp.w);

    draw_lists[DRAW_OPAQUE].draw(ctx);

    ez_end_rendering();
}

void Renderer::copy_to_screen(RenderContext* ctx)
{
    DebugLabel debug_label("Copy to screen", DebugLabel::WHITE);

    TextureRef* out_color_ref = ctx->get_texture_ref("out_color");
    TextureRef* scene_color_ref = ctx->get_texture_ref("scene_color");
    VkImageMemoryBarrier2 copy_barriers[] = {
        ez_image_barrier(scene_color_ref->get_texture(), EZ_RESOURCE_STATE_COPY_SOURCE),
        ez_image_barrier(out_color_ref->get_texture(), EZ_RESOURCE_STATE_COPY_DEST),
    };
    ez_pipeline_barrier(0, 0, nullptr, 2, copy_barriers);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.extent = { out_color_ref->get_desc().width, out_color_ref->get_desc().height, 1 };
    ez_copy_image(scene_color_ref->get_texture(), out_color_ref->get_texture(), copy_region);
}

Renderer* g_renderer = nullptr;