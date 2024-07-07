#pragma once

#include "render_constants.h"
#include <math/bounding_box.h>
#include <rhi/ez_vulkan.h>
#include <memory>

class VertexFactory;
class MaterialProxy;
struct Renderable
{
    int scene_index = -1;
    VkPrimitiveTopology primitive_topology;
    VertexFactory* vertex_factory;
    MaterialProxy* material_proxy;
    BoundingBox bounding_box;
};

struct RenderableCollector
{
    int renderable_count = 0;
    std::vector<Renderable> renderables;
    std::vector<SceneInstanceData> instance_datas;

    void clear();

    int add_renderable();

    Renderable* get_renderable(int idx);

    SceneInstanceData* get_scene_instance_data(int idx);
};