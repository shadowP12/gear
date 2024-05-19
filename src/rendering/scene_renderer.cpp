#include "scene_renderer.h"
#include "render_scene.h"
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