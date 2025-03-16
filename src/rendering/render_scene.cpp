#include "render_scene.h"
#include "render_context.h"
#include "render_system.h"
#include "vertex_factory.h"
#include "entity/entity.h"
#include <math/transform_util.h>

RenderScene::RenderScene()
{
}

RenderScene::~RenderScene()
{
}

RenderScene* g_scene = nullptr;