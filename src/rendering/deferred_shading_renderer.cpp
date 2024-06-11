#include "deferred_shading_renderer.h"
#include "render_scene.h"
#include "render_context.h"
#include "render_system.h"
#include "vertex_factory.h"
#include "material_proxy.h"

DeferredShadingRenderer::DeferredShadingRenderer()
    : SceneRenderer()
{
}

DeferredShadingRenderer::~DeferredShadingRenderer()
{
}

void DeferredShadingRenderer::render(RenderContext* ctx)
{
    SceneRenderer::render(ctx);
    fill_gbuffer(ctx);
    copy_to_screen(ctx);
}

void DeferredShadingRenderer::fill_gbuffer(RenderContext* ctx)
{
    ScopedDrawLabel draw_label("Fill gbuffer", ScopedDrawLabel::WHITE);

    TextureRef* out_color_ref = ctx->find_t_ref("out_color");
    uint32_t rt_width = out_color_ref->get_desc().width;
    uint32_t rt_height = out_color_ref->get_desc().height;

    bool is_new;
    EzTextureDesc texture_desc{};
    texture_desc.width = rt_width;
    texture_desc.height = rt_height;
    texture_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    texture_desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    TextureRef* gbuffer_0_ref = ctx->find_or_create_t_ref("gbuffer_0", texture_desc, is_new);
    if (is_new)
    {
        ez_create_texture_view(gbuffer_0_ref->get_texture(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    }

    TextureRef* gbuffer_1_ref = ctx->find_or_create_t_ref("gbuffer_1", texture_desc, is_new);
    if (is_new)
    {
        ez_create_texture_view(gbuffer_1_ref->get_texture(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    }

    texture_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    texture_desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    TextureRef* scene_color_ref = ctx->find_or_create_t_ref("scene_color", texture_desc, is_new);
    if (is_new)
    {
        ez_create_texture_view(scene_color_ref->get_texture(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    }

    texture_desc.format = VK_FORMAT_D24_UNORM_S8_UINT;
    texture_desc.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    TextureRef* scene_depth_ref = ctx->find_or_create_t_ref("scene_depth", texture_desc, is_new);
    if (is_new)
    {
        ez_create_texture_view(scene_depth_ref->get_texture(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1);
    }

    ez_reset_pipeline_state();

    VkImageMemoryBarrier2 rt_barriers[3];
    rt_barriers[0] = ez_image_barrier(gbuffer_0_ref->get_texture(), EZ_RESOURCE_STATE_RENDERTARGET);
    rt_barriers[1] = ez_image_barrier(gbuffer_1_ref->get_texture(), EZ_RESOURCE_STATE_RENDERTARGET);
    rt_barriers[2] = ez_image_barrier(scene_depth_ref->get_texture(), EZ_RESOURCE_STATE_DEPTH_WRITE);
    ez_pipeline_barrier(0, 0, nullptr, 3, rt_barriers);

    EzRenderingInfo rendering_info{};
    EzRenderingAttachmentInfo color_info{};
    color_info.texture = gbuffer_0_ref->get_texture();
    color_info.clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
    rendering_info.colors.push_back(color_info);

    color_info.texture = gbuffer_1_ref->get_texture();
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

    MaterialProxyPool* material_proxy_pool = RenderSystem::get()->get_material_proxy_pool();
    DrawCommandList& draw_list = scene->draw_list[DRAW_CMD_OPAQUE];
    for (int i = 0; i < draw_list.cmd_count; ++i)
    {
        DrawCommand& draw_cmd = draw_list.cmds[i];
        Renderable& renderable = scene->renderables[draw_cmd.renderable];
        RenderPrimitive& primitive = renderable.primitives[draw_cmd.primitive];
        MaterialProxy* material_proxy = material_proxy_pool->get_proxy(primitive.material_id);
    }

    ez_end_rendering();
}

void DeferredShadingRenderer::copy_to_screen(RenderContext* ctx)
{
    ScopedDrawLabel draw_label("Copy to screen", ScopedDrawLabel::WHITE);

    TextureRef* out_color_ref = ctx->find_t_ref("out_color");
    TextureRef* scene_color_ref = ctx->find_t_ref("scene_color");
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