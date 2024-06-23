#pragma once

#include "asset.h"
#include <rhi/ez_vulkan.h>
#include <math/bounding_box.h>

class Material;

struct MeshPrimitive
{
    uint32_t total_size;
    uint32_t total_offset;
    uint32_t vertex_count;
    uint32_t vertex_size;
    uint32_t vertex_offset;
    uint32_t index_count;
    uint32_t index_size;
    uint32_t index_offset;
    VkIndexType index_type;
    BoundingBox bounding_box;
    Material* material = nullptr;
    EzBuffer vertex_buffer = VK_NULL_HANDLE;
    EzBuffer index_buffer = VK_NULL_HANDLE;
    int vertex_factory;
    VkPrimitiveTopology primitive_topology;
};

class Mesh : public Asset
{
public:
    Mesh(const std::string& asset_path = "");

    virtual ~Mesh();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);
    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin);

    std::vector<uint8_t>& get_data() {return _data;}

    std::vector<MeshPrimitive*>& get_primitives() {return _primitives;}

private:
    void generate_mesh_buffers();

protected:
    friend class MeshPrimitive;
    std::vector<uint8_t> _data;
    std::vector<MeshPrimitive*> _primitives;
};