#include "render_system.h"
#include "render_context.h"
#include "render_scene.h"
#include "cluster_builder.h"
#include "clustered_forward_renderer.h"
#include "imgui_renderer.h"
#include "render_shared_data.h"
#include "window.h"
#include <core/memory.h>

void RenderSystem::setup()
{
    _ctx = new RenderContext();
    _scene = new RenderScene();
    _cluster_builder = new ClusterBuilder();
    _scene_renderer = new ClusteredForwardRenderer();
    _shared_data = new RenderSharedData();
    _imgui_renderer = new ImGuiRenderer();

    _scene_renderer->set_scene(_scene);
}

void RenderSystem::finish()
{
    SAFE_DELETE(_ctx);
    SAFE_DELETE(_scene_renderer);
    SAFE_DELETE(_cluster_builder);
    SAFE_DELETE(_scene);
    SAFE_DELETE(_shared_data);
    SAFE_DELETE(_imgui_renderer);
}

void RenderSystem::set_world(World* world)
{
    _scene->set_world(world);
}

void RenderSystem::render(Window* window)
{
    EzSwapchain swapchain = window->get_swapchain();
    if (!swapchain)
        return;

    ez_acquire_next_image(swapchain);

    _ctx->collect_viewport_info(window);

    {
        RenderContext::CreateStatus create_status;
        EzTextureDesc texture_desc{};
        texture_desc.width = swapchain->width;
        texture_desc.height = swapchain->height;
        texture_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
        texture_desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        TextureRef* t_ref = _ctx->create_texture_ref("out_color", texture_desc, create_status);
        if (create_status == RenderContext::CreateStatus::Recreated)
        {
            ez_create_texture_view(t_ref->get_texture(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
        }
    }

    _scene->prepare(_ctx);

    _scene_renderer->render(_ctx);

    _imgui_renderer->render(_ctx, window);

    // Copy to swapchain
    {
        TextureRef* t_ref = _ctx->get_texture_ref("out_color");
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

    // Present
    VkImageMemoryBarrier2 present_barrier[] = { ez_image_barrier(swapchain, EZ_RESOURCE_STATE_PRESENT) };
    ez_pipeline_barrier(0, 0, nullptr, 1, present_barrier);
    ez_present(swapchain);
}