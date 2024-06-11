#pragma once

#include "scene_renderer.h"

class DeferredShadingRenderer : public SceneRenderer
{
public:
    DeferredShadingRenderer();
    virtual ~DeferredShadingRenderer();

    void render(RenderContext* ctx) override;

protected:
    void fill_gbuffer(RenderContext* ctx);

    void copy_to_screen(RenderContext* ctx);
};