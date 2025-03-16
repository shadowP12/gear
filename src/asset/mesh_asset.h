#pragma once

#include "asset.h"
#include <rhi/ez_vulkan.h>
#include <math/bounding_box.h>

class MaterialAsset;

class MeshAsset : public Asset
{
public:
    MeshAsset(const std::string& asset_path = "");

    virtual ~MeshAsset();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);

    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin);

    std::vector<uint8_t>& get_data() { return _data; }

    struct Surface
    {
        uint32_t total_size;
        uint32_t total_offset;
        uint32_t vertex_count;
        uint32_t vertex_size;
        uint32_t vertex_offset;
        uint32_t position_size;
        uint32_t position_offset;
        uint32_t normal_size;
        uint32_t normal_offset;
        uint32_t tangent_size;
        uint32_t tangent_offset;
        uint32_t uv0_size;
        uint32_t uv0_offset;
        uint32_t uv1_size;
        uint32_t uv1_offset;
        uint32_t index_count;
        uint32_t index_size;
        uint32_t index_offset;
        bool using_16u;
        BoundingBox bounding_box;
        MaterialAsset* material = nullptr;
        VkPrimitiveTopology primitive_topology;
        int vertex_buffer_count = 0;
        EzBuffer vertex_buffers[EZ_NUM_VERTEX_BUFFERS];
        VkIndexType index_type;
        EzBuffer index_buffer = VK_NULL_HANDLE;
    };

    std::vector<Surface*>& get_surfaces() {return _surfaces;}

protected:
    std::vector<uint8_t> _data;
    std::vector<Surface*> _surfaces;
};