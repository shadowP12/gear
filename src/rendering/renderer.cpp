#include "renderer.h"
#include "render_scene.h"
#include "render_context.h"
#include "vertex_factory.h"
#include "utils/debug_utils.h"
#include "utils/render_utils.h"
#include "passes/light_cluster_pass.h"
#include "passes/post_process_pass.h"

Renderer::Renderer()
{
    light_cluster_pass = std::make_unique<LightClusterPass>();
    post_process_pass = std::make_unique<PostProcessPass>();
}

Renderer::~Renderer()
{
}

void Renderer::render(RenderContext* ctx)
{
    light_cluster_pass->predraw(ctx);

    prepare(ctx);
    light_cluster_pass->exec(ctx);
    render_opaque_pass(ctx);
    //light_cluster_pass->debug(ctx);
    post_process_pass->exec(ctx);
    copy_to_screen(ctx);
}

void Renderer::prepare(RenderContext* ctx)
{
    // Prepare RTs
    EzTexture out_color_rt = ctx->get_texture("out_color");
    uint32_t rt_width = out_color_rt->width;
    uint32_t rt_height = out_color_rt->height;

    RenderContext::CreateStatus create_status;
    EzTextureDesc texture_desc{};
    texture_desc.width = rt_width;
    texture_desc.height = rt_height;
    texture_desc.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    texture_desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    EzTexture scene_color_rt = ctx->create_texture("scene_color", texture_desc, create_status);
    if (create_status == RenderContext::CreateStatus::Recreated)
    {
        ez_create_texture_view(scene_color_rt, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    }

    texture_desc.format = VK_FORMAT_D24_UNORM_S8_UINT;
    texture_desc.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    EzTexture scene_depth_rt = ctx->create_texture("scene_depth", texture_desc, create_status);
    if (create_status == RenderContext::CreateStatus::Recreated)
    {
        ez_create_texture_view(scene_depth_rt, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1);
    }

    VkImageMemoryBarrier2 rt_barriers[2];
    rt_barriers[0] = ez_image_barrier(scene_color_rt, EZ_RESOURCE_STATE_RENDERTARGET);
    rt_barriers[1] = ez_image_barrier(scene_depth_rt, EZ_RESOURCE_STATE_DEPTH_WRITE);
    ez_pipeline_barrier(0, 0, nullptr, 2, rt_barriers);

    // Prepare FrameConstants
    RenderView* render_view = &g_scene->view[DISPLAY_VIEW];
    FrameConstants frame_constants;
    frame_constants.z_near_far = glm::vec2(render_view->zn, render_view->zf);
    frame_constants.cluster_size = ctx->cluster_size;
    frame_constants.view_position = glm::vec4(render_view->position, 0.0);
    frame_constants.view_matrix = render_view->view;
    frame_constants.proj_matrix = render_view->proj;
    frame_constants.inv_view_proj_matrix = glm::inverse(frame_constants.proj_matrix * frame_constants.view_matrix);
    frame_constants.ev100 = render_view->ev100;
    frame_constants.exposure = render_view->exposure;

    EzBufferDesc buffer_desc{};
    buffer_desc.size = sizeof(FrameConstants);
    buffer_desc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    EzBuffer frame_ub = ctx->create_buffer("u_frame", buffer_desc);
    update_render_buffer(frame_ub, EZ_RESOURCE_STATE_SHADER_RESOURCE, sizeof(FrameConstants), 0, (uint8_t*)&frame_constants);

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

    buffer_desc.size = sizeof(SceneInstanceData) * scene_data.size();
    EzBuffer scene_ub = ctx->create_buffer("u_scene", buffer_desc, false);
    update_render_buffer(scene_ub, EZ_RESOURCE_STATE_SHADER_RESOURCE, sizeof(SceneInstanceData) * scene_data.size(), 0, (uint8_t*)scene_data.data());
}

void Renderer::render_opaque_pass(RenderContext* ctx)
{
    DebugLabel debug_label("Render opaque pass", DebugLabel::WHITE);

    EzTexture scene_color_rt = ctx->get_texture("scene_color");
    EzTexture scene_depth_rt = ctx->get_texture("scene_depth");
    uint32_t rt_width = scene_color_rt->width;
    uint32_t rt_height = scene_color_rt->height;

    ez_reset_pipeline_state();

    EzRenderingInfo rendering_info{};
    EzRenderingAttachmentInfo color_info{};
    color_info.texture = scene_color_rt;
    color_info.clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
    rendering_info.colors.push_back(color_info);

    EzRenderingAttachmentInfo depth_info{};
    depth_info.texture = scene_depth_rt;
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

    EzTexture out_color_rt = ctx->get_texture("out_color");
    EzTexture final_rt_rt = post_process_pass->get_final_rt(ctx);
    VkImageMemoryBarrier2 copy_barriers[] = {
        ez_image_barrier(final_rt_rt, EZ_RESOURCE_STATE_COPY_SOURCE),
        ez_image_barrier(out_color_rt, EZ_RESOURCE_STATE_COPY_DEST),
    };
    ez_pipeline_barrier(0, 0, nullptr, 2, copy_barriers);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.extent = { out_color_rt->width, out_color_rt->height, 1 };
    ez_copy_image(final_rt_rt, out_color_rt, copy_region);
}

Renderer* g_renderer = nullptr;