#pragma once
#include "renderable.h"
#include "render_constants.h"
#include "render_resources.h"
#include "render_view.h"
#include "light.h"
#include "draw_command.h"
#include "collector/renderable_collector.h"
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

    void set_world(World* world);

    void prepare(RenderContext* ctx);

private:
    void clear_world();

    void init_world();

    void fill_draw_list(DrawCommandType type, int renderable_id);

public:
    // Views
    RenderView view[2];

    // Lights
    ObjectPool<OmniLight> point_lights;
    ObjectPool<OmniLight> spot_lights;
    ObjectPool<DirectionLight> dir_lights;

    // Collector begin
    RenderableCollector renderable_collector;
    SceneInstanceCollector scene_collector;
    // Collector end

    DrawCommandList draw_list[DRAW_CMD_MAX];

private:
    World* _world = nullptr;
};