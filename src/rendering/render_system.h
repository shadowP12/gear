#pragma once

#include <core/module.h>
#include <rhi/ez_vulkan.h>
#include <memory>

class RenderContext;
class SceneRenderer;
class MaterialProxyPool;

class RenderSystem : public Module<RenderSystem>
{
public:
    RenderSystem() = default;
    ~RenderSystem() = default;

    void setup();
    void finish();
    void execute(EzSwapchain swapchain);

    SceneRenderer* get_scene_renderer() { return _scene_renderer.get(); }
    MaterialProxyPool* get_material_proxy_pool() { return _material_proxy_pool.get(); }

private:
    std::shared_ptr<SceneRenderer> _scene_renderer;
    std::shared_ptr<MaterialProxyPool> _material_proxy_pool;
    std::shared_ptr<RenderContext> _ctx;
};