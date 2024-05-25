#pragma once

#include "scene_renderer.h"

class DeferredShadingRenderer : public SceneRenderer
{
public:
    DeferredShadingRenderer();
    virtual ~DeferredShadingRenderer();

    void render(RenderContext* ctx) override;
};