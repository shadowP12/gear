#include "scene_renderer.h"
#include "render_scene.h"
#include "render_context.h"
#include "asset/level.h"

SceneRenderer::SceneRenderer()
{
    scene = new RenderScene();
}

SceneRenderer::~SceneRenderer()
{
    delete scene;
}

void SceneRenderer::set_level(Level* level)
{
    scene->set_level(level);
}

void SceneRenderer::render(RenderContext* ctx)
{
    scene->prepare(ctx);
    update_frame_constants(ctx);
}

void SceneRenderer::update_frame_constants(RenderContext* ctx)
{
    _frame_constants.view_matrix = scene->view[1].view;
    _frame_constants.proj_matrix = scene->view[1].projection;

    UniformBuffer* frame_ub = ctx->find_ub(FRAME_CONSTANTS_NAME, sizeof(FrameConstants));
    frame_ub->write((uint8_t*)&_frame_constants, sizeof(FrameConstants));
}