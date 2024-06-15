#include "clustered_forward_renderer.h"
#include "render_scene.h"
#include "render_context.h"
#include "render_system.h"
#include "vertex_factory.h"
#include "material_proxy.h"

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
    copy_to_screen(ctx);
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