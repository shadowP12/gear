#pragma once

#include <rhi/ez_vulkan.h>
#include <unordered_map>

class VertexFactory
{
public:
    VertexFactory() = default;

    virtual ~VertexFactory();

    virtual int get_type_id() = 0;

    virtual void compilation_environment(std::vector<std::string>& macros) = 0;

    virtual void bind() = 0;

public:
    int vertex_count = 0;
    int vertex_buffer_count = 0;
    EzBuffer vertex_buffers[EZ_NUM_VERTEX_BUFFERS];
    int index_count = 0;
    VkIndexType index_type;
    EzBuffer index_buffer = VK_NULL_HANDLE;
};

#define STATIC_MESH_VERTEX_FACTORY 0
class StaticMeshVertexFactory : public VertexFactory
{
public:
    StaticMeshVertexFactory() = default;

    virtual ~StaticMeshVertexFactory();

    virtual int get_type_id() { return STATIC_MESH_VERTEX_FACTORY; };

    virtual void compilation_environment(std::vector<std::string>& macros);

    virtual void bind();
};