#pragma once

#include "renderable.h"
#include "render_constants.h"
#include "render_resources.h"
#include "draw_command.h"
#include <core/event.h>
#include <core/memory.h>
#include <vector>

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

    void bind(int scene_idx);

private:
    void clear_world();

    void init_world();

protected:
    void fill_draw_list(DrawCommandType type, int renderable_id);

public:
    RenderView view[2]; // Main / Display
    int renderable_count = 0;
    std::vector<Renderable> renderables;
    std::vector<SceneInstanceData> instance_datas;
    DrawCommandList draw_list[DRAW_CMD_MAX];
    UniformBuffer* scene_ub = nullptr;

private:
    World* _world = nullptr;
};