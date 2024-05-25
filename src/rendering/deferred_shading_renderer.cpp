#include "deferred_shading_renderer.h"

DeferredShadingRenderer::DeferredShadingRenderer()
    : SceneRenderer()
{
}

DeferredShadingRenderer::~DeferredShadingRenderer()
{
}

void DeferredShadingRenderer::render(RenderContext* ctx)
{
    SceneRenderer::render(ctx);
}