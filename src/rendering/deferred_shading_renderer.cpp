#include "deferred_shading_renderer.h"
#include "render_scene.h"

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
    fill_gbuffer(ctx);
}

void DeferredShadingRenderer::fill_gbuffer(RenderContext* ctx)
{

}