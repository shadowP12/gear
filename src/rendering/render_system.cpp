#include "render_system.h"
#include "render_context.h"
#include "render_scene.h"
#include "renderer.h"
#include "imgui_renderer.h"
#include "render_shared_data.h"
#include "window.h"
#include <core/memory.h>

void RenderSystem::setup()
{
    _ctx = new RenderContext();
    g_rsd = new RenderSharedData();
    g_scene = new RenderScene();
    g_renderer = new Renderer();
    g_imgui_renderer = new ImGuiRenderer();
}

void RenderSystem::finish()
{
    SAFE_DELETE(g_scene);
    SAFE_DELETE(g_imgui_renderer);
    SAFE_DELETE(g_renderer);
    SAFE_DELETE(g_rsd);
    SAFE_DELETE(_ctx);
}

void RenderSystem::render(Window* window)
{
    predraw_event.broadcast();

    EzSwapchain swapchain = window->get_swapchain();
    if (!swapchain)
        return;

    ez_acquire_next_image(swapchain);

    _ctx->collect_info(window);

    {
        RenderContext::CreateStatus create_status;
        EzTextureDesc texture_desc{};
        texture_desc.width = swapchain->width;
        texture_desc.height = swapchain->height;
        texture_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
        texture_desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        EzTexture out_rt = _ctx->create_texture("out_color", texture_desc, create_status);
        if (create_status == RenderContext::CreateStatus::Recreated)
        {
            ez_create_texture_view(out_rt, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
        }
    }

    g_renderer->render(_ctx);

    g_imgui_renderer->render(_ctx, window);

    // Copy to swapchain
    {
        EzTexture out_rt = _ctx->get_texture("out_color");
        VkImageMemoryBarrier2 copy_barriers[] = {
            ez_image_barrier(out_rt, EZ_RESOURCE_STATE_COPY_SOURCE),
            ez_image_barrier(swapchain, EZ_RESOURCE_STATE_COPY_DEST),
        };
        ez_pipeline_barrier(0, 0, nullptr, 2, copy_barriers);

        VkImageCopy copy_region = {};
        copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.srcSubresource.layerCount = 1;
        copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.dstSubresource.layerCount = 1;
        copy_region.extent = { swapchain->width, swapchain->height, 1 };
        ez_copy_image(out_rt, swapchain, copy_region);
    }

    // Present
    VkImageMemoryBarrier2 present_barrier[] = { ez_image_barrier(swapchain, EZ_RESOURCE_STATE_PRESENT) };
    ez_pipeline_barrier(0, 0, nullptr, 1, present_barrier);
    ez_present(swapchain);
}