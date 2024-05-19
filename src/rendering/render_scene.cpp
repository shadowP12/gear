#include "render_scene.h"
#include "asset/level.h"
#include "entity/entity.h"

RenderScene::RenderScene()
{}

RenderScene::~RenderScene()
{}

void RenderScene::set_level(Level* level)
{
    _level = level;
}