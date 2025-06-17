#include "texture_asset.h"

#include <core/path.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBIR_FLAG_ALPHA_PREMULTIPLIED
#include <stb_image.h>

Image* load_image(const std::string& file)
{
    if (Path::extension(file) == "png")
    {
        int width, height, channels;
        unsigned char* pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        Image* image = new Image;
        image->width = width;
        image->height = height;
        image->data_size = width * height * 4;
        image->data = new uint8_t[image->data_size];
        memcpy(image->data, pixels, image->data_size);
        image->format = VK_FORMAT_R8G8B8A8_UNORM;
        stbi_image_free(pixels);

        return image;
    }
    return nullptr;
}

EzTexture create_texture(Image* image)
{
    EzTexture texture = VK_NULL_HANDLE;
    EzTextureDesc texture_desc{};
    texture_desc.width = image->width;
    texture_desc.height = image->height;
    texture_desc.format = image->format;
    ez_create_texture(texture_desc, texture);
    ez_create_texture_view(texture, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

    VkImageMemoryBarrier2 barrier = ez_image_barrier(texture, EZ_RESOURCE_STATE_COPY_DEST);
    ez_pipeline_barrier(0, 0, nullptr, 1, &barrier);

    VkBufferImageCopy range{};
    range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.imageSubresource.baseArrayLayer = 0;
    range.imageSubresource.layerCount = 1;
    range.imageExtent.width = image->width;
    range.imageExtent.height = image->height;
    range.imageExtent.depth = image->depth;
    ez_update_image(texture, range, image->data);

    barrier = ez_image_barrier(texture, EZ_RESOURCE_STATE_SHADER_RESOURCE | EZ_RESOURCE_STATE_UNORDERED_ACCESS);
    ez_pipeline_barrier(0, 0, nullptr, 1, &barrier);

    return texture;
}

TextureAsset::TextureAsset(const std::string& asset_path)
    : Asset(asset_path) {}

TextureAsset::~TextureAsset()
{
    if (_image)
    {
        delete[] _image->data;
        delete _image;
    }

    if (_texture)
    {
        ez_destroy_texture(_texture);
    }
}

void TextureAsset::serialize(SerializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.object([&]() {
        ctx.field("uri", _uri);
    });
}

void TextureAsset::deserialize(DeserializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.field("uri", _uri);
    _image = load_image(Path::fix_path(_uri));

    if (_image)
    {
        _texture = create_texture(_image);
        ez_create_texture_view(_texture, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    }
}