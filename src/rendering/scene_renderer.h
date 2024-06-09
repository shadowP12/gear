#pragma once

#include "render_constants.h"
#include "render_resources.h"
#include <memory>

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
    std::shared_ptr<RenderScene> scene;
    FrameConstants frame_constants;
    std::shared_ptr<UniformBuffer> frame_ub;
};