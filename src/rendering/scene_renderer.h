#pragma once

#include "render_constants.h"

class Level;
class RenderScene;
class RenderContext;
class SceneRenderer
{
public:
    SceneRenderer();
    virtual ~SceneRenderer();

    void set_level(Level* level);

    virtual void render(RenderContext* ctx);

protected:
    void update_frame_constants(RenderContext* ctx);

protected:
    RenderScene* scene;
    FrameConstants _frame_constants;
};