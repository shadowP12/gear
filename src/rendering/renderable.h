#pragma once

#include "render_resources.h"
#include <math/bounding_box.h>
#include <rhi/ez_vulkan.h>
#include <memory>

class MaterialProxy;
struct Renderable
{
    int scene_index = -1;
    int vertex_factory;
    int vertex_count;
    int index_count;
    VkIndexType index_type;
    EzBuffer vertex_buffer = VK_NULL_HANDLE;
    EzBuffer index_buffer = VK_NULL_HANDLE;
    VkPrimitiveTopology primitive_topology;
    MaterialProxy* material_proxy;
    BoundingBox bounding_box;
};