#include "renderable.h"

void RenderableCollector::clear()
{
    renderable_count = 0;
}

int RenderableCollector::add_renderable()
{
    if (renderable_count >= renderables.size())
    {
        renderables.emplace_back();
        instance_datas.emplace_back();
    }

    renderables[renderable_count].scene_index = renderable_count;

    return renderable_count++;
}

Renderable* RenderableCollector::get_renderable(int idx)
{
    return &renderables[idx];
}

SceneInstanceData* RenderableCollector::get_scene_instance_data(int idx)
{
    return &instance_datas[idx];
}