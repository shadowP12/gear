#include "clustered_forward_renderer.h"
#include "render_scene.h"
#include "render_context.h"
#include "render_system.h"
#include "vertex_factory.h"
#include "material_proxy.h"
#include "shader_builder.h"
#include "sampler_pool.h"

ClusteredForwardRenderer::ClusteredForwardRenderer()
    : SceneRenderer()
{
}

ClusteredForwardRenderer::~ClusteredForwardRenderer()
{
}

void ClusteredForwardRenderer::render(RenderContext* ctx)
{
    SceneRenderer::render(ctx);
    prepare(ctx);
    render_opaque_pass(ctx);
    copy_to_screen(ctx);
}

void ClusteredForwardRenderer::render_list(const DrawCommandType& draw_type)
{
    SamplerPool* sampler_pool = RenderSystem::get()->get_sampler_pool();
    DrawCommandList& draw_list = scene->draw_list[draw_type];
    for (int i = 0; i < draw_list.cmd_count; ++i)
    {
        DrawCommand& draw_cmd = draw_list.cmds[i];
        Renderable& renderable = scene->renderables[draw_cmd.renderable];
        VertexFactory* vertex_factory = renderable.vertex_factory;
        MaterialProxy* material_proxy = renderable.material_proxy;

        ShaderBuilder vs_builder;
        vs_builder.set_source("shader://clustered_forward.vert");
        vs_builder.set_vertex_factory(vertex_factory);
        vs_builder.set_material_proxy(material_proxy);

        ShaderBuilder fs_builder;
        fs_builder.set_source("shader://clustered_forward.frag");
        fs_builder.set_vertex_factory(vertex_factory);
        fs_builder.set_material_proxy(material_proxy);

        ez_set_vertex_shader(vs_builder.build());
        ez_set_fragment_shader(fs_builder.build());

        ez_bind_buffer(0, frame_ub->get_buffer());

        scene->bind(renderable.scene_index);

        sampler_pool->bind();

        material_proxy->bind();

        vertex_factory->bind();

        ez_set_primitive_topology(renderable.primitive_topology);

        ez_draw_indexed(vertex_factory->index_count, 0, 0);
    }
}

void ClusteredForwardRenderer::prepare(RenderContext* ctx)
{
    TextureRef* out_color_ref = ctx->find_texture_ref("out_color");
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
}

void ClusteredForwardRenderer::render_opaque_pass(RenderContext* ctx)
{
    DrawLabel draw_label("Render opaque pass", DrawLabel::WHITE);

    TextureRef* scene_color_ref = ctx->find_texture_ref("scene_color");
    TextureRef* scene_depth_ref = ctx->find_texture_ref("scene_depth");

    uint32_t rt_width = scene_color_ref->get_desc().width;
    uint32_t rt_height = scene_color_ref->get_desc().height;

    VkImageMemoryBarrier2 rt_barriers[2];
    rt_barriers[0] = ez_image_barrier(scene_color_ref->get_texture(), EZ_RESOURCE_STATE_RENDERTARGET);
    rt_barriers[1] = ez_image_barrier(scene_depth_ref->get_texture(), EZ_RESOURCE_STATE_DEPTH_WRITE);
    ez_pipeline_barrier(0, 0, nullptr, 2, rt_barriers);

    ez_reset_pipeline_state();

    EzRenderingInfo rendering_info{};
    EzRenderingAttachmentInfo color_info{};
    color_info.texture = scene_color_ref->get_texture();
    color_info.clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
    rendering_info.colors.push_back(color_info);

    EzRenderingAttachmentInfo depth_info{};
    depth_info.texture = scene_depth_ref->get_texture();
    depth_info.clear_value.depthStencil = {1.0f, 1};
    rendering_info.width = rt_width;
    rendering_info.height = rt_height;
    rendering_info.depth.push_back(depth_info);

    ez_begin_rendering(rendering_info);

    ez_set_viewport(0, 0, (float)rt_width, (float)rt_height);
    ez_set_scissor(0, 0, (int32_t)rt_width, (int32_t)rt_height);

    render_list(DRAW_CMD_OPAQUE);

    ez_end_rendering();
}

void ClusteredForwardRenderer::copy_to_screen(RenderContext* ctx)
{
    DrawLabel draw_label("Copy to screen", DrawLabel::WHITE);

    TextureRef* out_color_ref = ctx->find_texture_ref("out_color");
    TextureRef* scene_color_ref = ctx->find_texture_ref("scene_color");
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