#include "vertex_factory.h"

VertexFactory::~VertexFactory()
{
}

StaticMeshVertexFactory::~StaticMeshVertexFactory()
{
}

void StaticMeshVertexFactory::compilation_environment(std::vector<std::string>& macros)
{
    macros.push_back("STATIC_MESH_VERTEX_FACTORY");
}

void StaticMeshVertexFactory::bind()
{
    ez_set_vertex_binding(0, 12);
    ez_set_vertex_attrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT);

    ez_set_vertex_binding(1, 12);
    ez_set_vertex_attrib(1, 1, VK_FORMAT_R32G32B32_SFLOAT);

    ez_set_vertex_binding(2, 12);
    ez_set_vertex_attrib(2, 2, VK_FORMAT_R32G32B32_SFLOAT);

    ez_set_vertex_binding(3, 8);
    ez_set_vertex_attrib(3, 3, VK_FORMAT_R32G32_SFLOAT);

    ez_bind_vertex_buffers(0, vertex_buffer_count, vertex_buffers);

    ez_bind_index_buffer(index_buffer, index_type);
}