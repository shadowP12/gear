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
    void execute(float dt);

private:
    RenderContext* _ctx;
    SceneRenderer* _scene_renderer;
};