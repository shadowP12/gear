#include "render_system.h"
#include "render_context.h"
#include "render_scene.h"
#include "deferred_shading_renderer.h"
#include "material_proxy.h"

void RenderSystem::setup()
{
    _ctx = std::make_shared<RenderContext>();
    _scene_renderer = std::make_shared<DeferredShadingRenderer>();
    _material_proxy_pool = std::make_shared<MaterialProxyPool>();
}

void RenderSystem::finish()
{
}

void RenderSystem::execute(float dt)
{
    _ctx->update(dt);
    _material_proxy_pool->update_dirty_proxys();
    _scene_renderer->render(_ctx.get());
}