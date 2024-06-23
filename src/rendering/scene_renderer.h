#pragma once

#include "render_constants.h"
#include "render_resources.h"

class World;
class RenderScene;
class RenderContext;
class SceneRenderer
{
public:
    SceneRenderer();
    virtual ~SceneRenderer();

    void set_world(World* world);

    virtual void render(RenderContext* ctx);

protected:
    void update_frame_constants(RenderContext* ctx);

protected:
    RenderScene* scene = nullptr;
    FrameConstants frame_constants;
    UniformBuffer* frame_ub = nullptr;
};