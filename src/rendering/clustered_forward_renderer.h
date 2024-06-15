#pragma once

#include "scene_renderer.h"

class ClusteredForwardRenderer : public SceneRenderer
{
public:
    ClusteredForwardRenderer();
    virtual ~ClusteredForwardRenderer();

    void render(RenderContext* ctx) override;

protected:
    void prepare(RenderContext* ctx);

    void copy_to_screen(RenderContext* ctx);
};