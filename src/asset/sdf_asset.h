#pragma once

#include "asset.h"

#include <math/bounding_box.h>
#include <rhi/ez_vulkan.h>

class SDFAsset : public Asset
{
public:
    SDFAsset(const std::string& asset_path = "");

    virtual ~SDFAsset();

    virtual void serialize(SerializationContext& ctx, BinaryStream& bin_stream);

    virtual void deserialize(DeserializationContext& ctx, BinaryStream& bin_stream);

    glm::vec3 get_local_to_uvw_mul() { return _local_to_uvw_mul; }

    glm::vec3 get_local_to_uvw_add() { return _local_to_uvw_add; }

    uint32_t get_resolution() { return _resolution; }

    BoundingBox get_bounds() { return _bounds; }

    EzTexture get_texture() { return _texture; }

protected:
    std::vector<uint8_t> _data;
    glm::vec3 _local_to_uvw_mul;
    glm::vec3 _local_to_uvw_add;
    uint32_t _resolution;
    BoundingBox _bounds;
    EzTexture _texture;
};