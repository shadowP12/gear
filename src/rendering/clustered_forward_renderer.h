#pragma once

#include "scene_renderer.h"
#include "draw_command.h"

class ClusteredForwardRenderer : public SceneRenderer
{
public:
    ClusteredForwardRenderer();
    virtual ~ClusteredForwardRenderer();

    void render(RenderContext* ctx) override;

protected:
    void render_list(const DrawCommandType& draw_type);

    void prepare(RenderContext* ctx);

    void render_opaque_pass(RenderContext* ctx);

    void copy_to_screen(RenderContext* ctx);
};