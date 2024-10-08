#pragma once

#include <core/module.h>
#include <rhi/ez_vulkan.h>
#include <memory>

class World;
class Window;
class RenderScene;
class RenderContext;
class RenderSharedData;
class ImGuiRenderer;
class ClusterBuilder;
class ClusteredForwardRenderer;

class RenderSystem : public Module<RenderSystem>
{
public:
    RenderSystem() = default;
    ~RenderSystem() = default;

    void setup();
    void finish();
    void render(Window* window);

    void set_world(World* world);

    RenderScene* get_scene() { return _scene; }
    RenderSharedData* get_shared_data() { return _shared_data; }
    ClusterBuilder* get_cluster_builder() { return _cluster_builder; }
    ClusteredForwardRenderer* get_scene_renderer() { return _scene_renderer; }

private:
    RenderContext* _ctx;
    RenderScene* _scene;
    RenderSharedData* _shared_data;
    ClusterBuilder* _cluster_builder;
    ClusteredForwardRenderer* _scene_renderer;
    ImGuiRenderer* _imgui_renderer;
};