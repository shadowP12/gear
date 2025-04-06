#pragma once

#include "render_constants.h"
#include "draw_command.h"
#include <memory>

class RenderContext;

class Renderer
{
public:
    Renderer();

    ~Renderer();

    void render(RenderContext* ctx);

    void prepare(RenderContext* ctx);

    void render_opaque_pass(RenderContext* ctx);

    void copy_to_screen(RenderContext* ctx);

    DrawCommandList draw_lists[DRAW_MAXCOUNT];

    std::unique_ptr<class LightClusterPass> light_cluster_pass;
    std::unique_ptr<class PostProcessPass> post_process_pass;
};

extern Renderer* g_renderer;