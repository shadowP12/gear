#include "scene_renderer.h"
#include "render_scene.h"
#include "render_context.h"
#include "asset/level.h"

SceneRenderer::SceneRenderer()
{
    scene = std::make_unique<RenderScene>();
}

SceneRenderer::~SceneRenderer()
{
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
    frame_constants.view_matrix = scene->view[1].view;
    frame_constants.proj_matrix = scene->view[1].projection;

    if (!frame_ub)
    {
        frame_ub = std::make_shared<UniformBuffer>(sizeof(FrameConstants));
    }
    frame_ub->write((uint8_t*)&frame_constants, sizeof(FrameConstants));
}