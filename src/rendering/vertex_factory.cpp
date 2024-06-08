#include "vertex_factory.h"

static std::unordered_map<int, EzVertexBinding> vf_pool;

void add_vertex_attrib(EzVertexBinding& layout, uint32_t location, VkFormat format, uint32_t& offset)
{
    layout.vertex_stride += ez_get_format_stride(format);
    layout.vertex_attribs[location].format = format;
    layout.vertex_attribs[location].offset = offset;
    layout.vertex_attrib_mask |= 1 << location;
    offset = layout.vertex_stride;
}

struct StaticMeshVertexFactoryRegister
{
    StaticMeshVertexFactoryRegister()
    {
        uint32_t offset = 0;
        EzVertexBinding layout{};
        add_vertex_attrib(layout, 0, VK_FORMAT_R32G32B32_SFLOAT, offset);
        add_vertex_attrib(layout, 1, VK_FORMAT_R32G32B32_SFLOAT, offset);
        add_vertex_attrib(layout, 2, VK_FORMAT_R32G32B32_SFLOAT, offset);
        add_vertex_attrib(layout, 3, VK_FORMAT_R32G32_SFLOAT, offset);
        vf_pool[STATIC_MESH_VERTEX_FACTORY] = layout;
    }
};
StaticMeshVertexFactoryRegister static_mesh_vf_register;

EzVertexBinding& get_vertex_factory_layout(int id)
{
    return vf_pool[id];
}