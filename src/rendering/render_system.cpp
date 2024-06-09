#include "render_system.h"
#include "render_context.h"
#include "render_scene.h"
#include "deferred_shading_renderer.h"
#include "material_proxy.h"

void RenderSystem::setup()
{
    _ctx = std::make_shared<RenderContext>();
    _scene_renderer = std::make_shared<DeferredShadingRenderer>();
    _material_proxy_pool = std::make_shared<MaterialProxyPool>();
}

void RenderSystem::finish()
{
    _ctx.reset();
    _scene_renderer.reset();
    _material_proxy_pool.reset();
}

void RenderSystem::execute(float dt, EzSwapchain swapchain)
{
    _ctx->update(dt);

    {
        bool is_new;
        EzTextureDesc texture_desc{};
        texture_desc.width = swapchain->width;
        texture_desc.height = swapchain->height;
        texture_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
        texture_desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        TextureRef* t_ref = _ctx->find_or_create_t_ref("out_color", texture_desc, is_new);
        if (is_new)
        {
            ez_create_texture_view(t_ref->get_texture(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
        }
    }

    _material_proxy_pool->update_dirty_proxys();
    _scene_renderer->render(_ctx.get());

    // Copy to swapchain
    {
        TextureRef* t_ref = _ctx->find_t_ref("out_color");
        VkImageMemoryBarrier2 copy_barriers[] = {
            ez_image_barrier(t_ref->get_texture(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT),
            ez_image_barrier(swapchain, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT),
        };
        ez_pipeline_barrier(0, 0, nullptr, 2, copy_barriers);

        VkImageCopy copy_region = {};
        copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.srcSubresource.layerCount = 1;
        copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.dstSubresource.layerCount = 1;
        copy_region.extent = { swapchain->width, swapchain->height, 1 };
        ez_copy_image(t_ref->get_texture(), swapchain, copy_region);
    }
}