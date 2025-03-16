#pragma once

#include <rhi/ez_vulkan.h>

class VertexFactory
{
public:
    EzVertexLayout layout;
    int instance_count = 1;
    int vertex_count = 0;
    int vertex_buffer_count = 0;
    EzBuffer vertex_buffers[EZ_NUM_VERTEX_BUFFERS];
    int index_count = 0;
    VkIndexType index_type;
    EzBuffer index_buffer = VK_NULL_HANDLE;
    VkPrimitiveTopology prim_topo;
};