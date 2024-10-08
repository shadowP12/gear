#pragma once

#include "render_constants.h"
#include "draw_command.h"
#include "cluster_builder.h"

class RenderScene;
class RenderContext;

class ClusteredForwardRenderer
{
public:
    ClusteredForwardRenderer();
    ~ClusteredForwardRenderer();

    void set_scene(RenderScene* scene);

    void render(RenderContext* ctx);

protected:
    void render_list(RenderContext* ctx, const DrawCommandType& draw_type);

    void prepare(RenderContext* ctx);

    void render_opaque_pass(RenderContext* ctx);

    void copy_to_screen(RenderContext* ctx);

private:
    RenderScene* _scene = nullptr;
    FrameConstants _frame_constants;
};