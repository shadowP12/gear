#pragma once
#include "draw_command.h"
#include "light.h"
#include "render_constants.h"
#include "render_view.h"
#include "renderable.h"
#include <core/event.h>
#include <core/memory.h>
#include <core/object_pool.h>
#include <vector>

#define MAIN_VIEW 0
#define DISPLAY_VIEW 1

class World;
class Entity;
class RenderContext;

class RenderScene
{
public:
    RenderScene();

    ~RenderScene();

public:
    // Views
    RenderView view[2];

    // Lights
    ObjectPool<PunctualLight> point_lights;
    ObjectPool<PunctualLight> spot_lights;
    ObjectPool<DirectionLight> dir_lights;

    // Renderables
    ObjectPool<Renderable> renderables;

    // Scene instances
    ObjectPool<SceneInstanceData> scene_insts;
};

extern RenderScene* g_scene;