#pragma once

#include "asset.h"

#include <rhi/ez_vulkan.h>

struct Image {
    uint8_t* data = nullptr;
    uint32_t data_size = 0;
    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t depth = 1;
    VkFormat format;
};

EzTexture create_texture(Image* image);

class TextureAsset : public Asset
{
public:
    TextureAsset(const std::string& asset_path = "");

    virtual ~TextureAsset();

    virtual void serialize(SerializationContext& ctx, BinaryStream& bin_stream);

    virtual void deserialize(DeserializationContext& ctx, BinaryStream& bin_stream);

    Image* get_image() { return _image; }

    EzTexture get_texture() { return _texture; }

private:
    std::string _uri;
    Image* _image = nullptr;
    EzTexture _texture = VK_NULL_HANDLE;
};