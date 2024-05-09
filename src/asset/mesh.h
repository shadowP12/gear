#pragma once

#include "asset.h"
#include <rhi/ez_vulkan.h>
#include <math/bounding_box.h>

struct SubMesh
{
    uint32_t total_size;
    uint32_t total_offset;
    uint32_t vertex_count;
    uint32_t vertex_size;
    uint32_t position_size;
    uint32_t position_offset;
    uint32_t normal_size;
    uint32_t normal_offset;
    uint32_t tangent_size;
    uint32_t tangent_offset;
    uint32_t uv_size;
    uint32_t uv_offset;
    uint32_t index_count;
    uint32_t index_size;
    uint32_t index_offset;
    VkIndexType index_type;
    BoundingBox bounding_box;
};

class Mesh : public Asset
{
public:
    Mesh(const std::string& asset_path = "");

    virtual ~Mesh();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, std::vector<uint8_t>& bin);
    virtual void deserialize(const rapidjson::Value& reader, const std::vector<uint8_t>& bin);

    std::vector<uint8_t>& get_data() {return _data;}

    std::vector<SubMesh*>& get_sub_meshes() {return _sub_meshes;}

protected:
    friend class SubMesh;
    std::vector<uint8_t> _data;
    std::vector<SubMesh*> _sub_meshes;
};