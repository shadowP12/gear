#include "deferred_shading_renderer.h"
#include "render_scene.h"
#include "render_context.h"

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
}

void DeferredShadingRenderer::fill_gbuffer(RenderContext* ctx)
{
    TextureRef* out_color_ref = ctx->find_t_ref("out_color");

    bool is_new;
    EzTextureDesc texture_desc{};
    texture_desc.width = out_color_ref->get_desc().width;
    texture_desc.height = out_color_ref->get_desc().width;
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


}