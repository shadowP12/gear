#include "sdf_asset.h"

SdfAsset::SdfAsset(const std::string& asset_path)
    : Asset(asset_path) {}

SdfAsset::~SdfAsset()
{
    if (_texture)
    {
        ez_destroy_texture(_texture);
    }
}

void SdfAsset::serialize(SerializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.object([&]() {
        ctx.field("data_size", _data.size());
        ctx.field("resolution", _resolution);
        ctx.field("local_to_uvw_mul", _local_to_uvw_mul);
        ctx.field("local_to_uvw_add", _local_to_uvw_add);
        ctx.field("bounds", _bounds);
    });

    bin_stream.write(_data.data(), _data.size());
}

void SdfAsset::deserialize(DeserializationContext& ctx, BinaryStream& bin_stream)
{
    uint32_t data_size;
    ctx.field("data_size", data_size);
    uint8_t* data = bin_stream.read(data_size);
    _data.insert(_data.end(), data, data + data_size);

    ctx.field("resolution", _resolution);
    ctx.field("local_to_uvw_mul", _local_to_uvw_mul);
    ctx.field("local_to_uvw_add", _local_to_uvw_add);
    ctx.field("bounds", _bounds);

    // Create rhi texture
    EzTextureDesc texture_desc{};
    texture_desc.width = _resolution;
    texture_desc.height = _resolution;
    texture_desc.depth = _resolution;
    texture_desc.image_type = VK_IMAGE_TYPE_3D;
    texture_desc.format = VK_FORMAT_R32_SFLOAT;
    ez_create_texture(texture_desc, _texture);
    ez_create_texture_view(_texture, VK_IMAGE_VIEW_TYPE_3D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

    VkImageMemoryBarrier2 barrier = ez_image_barrier(_texture, EZ_RESOURCE_STATE_COPY_DEST);
    ez_pipeline_barrier(0, 0, nullptr, 1, &barrier);

    VkBufferImageCopy range{};
    range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.imageSubresource.baseArrayLayer = 0;
    range.imageSubresource.layerCount = 1;
    range.imageExtent.width = _resolution;
    range.imageExtent.height = _resolution;
    range.imageExtent.depth = _resolution;
    ez_update_image(_texture, range, _data.data());

    barrier = ez_image_barrier(_texture, EZ_RESOURCE_STATE_SHADER_RESOURCE | EZ_RESOURCE_STATE_UNORDERED_ACCESS);
    ez_pipeline_barrier(0, 0, nullptr, 1, &barrier);
}