#include "render_system.h"
#include "render_context.h"
#include "render_scene.h"
#include "deferred_shading_renderer.h"

void RenderSystem::setup()
{
    _ctx = new RenderContext();
    _scene_renderer = new DeferredShadingRenderer();
}

void RenderSystem::finish()
{
    delete _ctx;
    delete _scene_renderer;
}