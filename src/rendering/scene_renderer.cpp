#include "scene_renderer.h"
#include "render_scene.h"
#include "render_context.h"
#include "world.h"
#include <core/memory.h>

SceneRenderer::SceneRenderer()
{
    scene = new RenderScene();
}

SceneRenderer::~SceneRenderer()
{
    if (frame_ub)
        SAFE_DELETE(frame_ub);
    SAFE_DELETE(scene);
}

void SceneRenderer::set_world(World* world)
{
    scene->set_world(world);
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
        frame_ub = new UniformBuffer(sizeof(FrameConstants));
    }
    frame_ub->write((uint8_t*)&frame_constants, sizeof(FrameConstants));
}