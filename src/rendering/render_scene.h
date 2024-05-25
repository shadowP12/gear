#pragma once

#include "renderable.h"
#include "render_constants.h"
#include <core/event.h>
#include <vector>

class Level;
class Entity;
class RenderContext;
class RenderScene
{
public:
    RenderScene();
    ~RenderScene();

    void set_level(Level* level);

    void prepare(RenderContext* ctx);

private:
    void clear_scene();

    void init_scene();

    void notify_received(int what, int id);

    void transform_changed(int id);

    void camera_changed(int id);

public:
    RenderView view[2]; // Main/Display
    std::vector<Renderable> renderables;
    std::vector<SceneTransform> scene_transforms;

private:
    Level* _level = nullptr;
    EventHandle _notify_handle;
    std::vector<int> _upload_transform_indices;

    struct LevelMapping
    {
        int rb = -1;    // Renderable
        int view = -1;  // RenderView
    };
    std::vector<LevelMapping> _level_mappings;
};