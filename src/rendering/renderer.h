#pragma once

#include "render_constants.h"
#include <memory>

class RenderContext;

class Renderer
{
public:
    Renderer();

    ~Renderer();

    void render(RenderContext* ctx);

    void setup(RenderContext* ctx);

    void render_opaque_pass(RenderContext* ctx);

    void copy_to_screen(RenderContext* ctx);

    std::unique_ptr<class LightClusterPass> light_cluster_pass;
    std::unique_ptr<class ShadowPass> shadow_pass;
    std::unique_ptr<class PostProcessPass> post_process_pass;
};

extern Renderer* g_renderer;