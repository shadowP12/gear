#pragma once

#include <core/module.h>

class RenderContext;
class SceneRenderer;

class RenderSystem : public Module<RenderSystem>
{
public:
    RenderSystem() = default;
    ~RenderSystem() = default;

    void setup();
    void finish();

private:
    RenderContext* _ctx;
    SceneRenderer* _scene_renderer;
};