#pragma once

#include <core/module.h>
#include <rhi/ez_vulkan.h>
#include <memory>

class Window;
class RenderContext;
class SceneRenderer;
class ImGuiRenderer;
class SamplerPool;

class RenderSystem : public Module<RenderSystem>
{
public:
    RenderSystem() = default;
    ~RenderSystem() = default;

    void setup();
    void finish();
    void render(Window* window);

    SamplerPool* get_sampler_pool() { return _sampler_pool; }
    SceneRenderer* get_scene_renderer() { return _scene_renderer; }

private:
    SamplerPool* _sampler_pool;
    SceneRenderer* _scene_renderer;
    ImGuiRenderer* _imgui_renderer;
    RenderContext* _ctx;
};