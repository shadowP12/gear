#pragma once

#include "render_constants.h"
#include "draw_command.h"

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
};

extern Renderer* g_renderer;