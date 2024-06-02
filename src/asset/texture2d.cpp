#include "texture2d.h"
#include "image.h"
#include "image_utilities.h"

Texture2D::Texture2D(const std::string& asset_path)
    : Asset(asset_path)
{
}

Texture2D::~Texture2D()
{
    if (_image)
        delete _image;

    if (_texture)
        ez_destroy_texture(_texture);
}

void Texture2D::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("uri");
    writer.String(_uri.c_str());
    writer.EndObject();
}

void Texture2D::deserialize(const rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    _uri = value["uri"].GetString();

    generate_texture();
}

void Texture2D::generate_texture()
{
    _image = ImageUtilities::load_image(_uri);

    if (_image)
    {
        EzTextureDesc texture_desc{};
        texture_desc.width = _image->width;
        texture_desc.height = _image->height;
        texture_desc.format = _image->format;
        ez_create_texture(texture_desc, _texture);
        ez_create_texture_view(_texture, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

        VkImageMemoryBarrier2 barrier = ez_image_barrier(_texture, EZ_RESOURCE_STATE_COPY_DEST);
        ez_pipeline_barrier(0, 0, nullptr, 1, &barrier);

        VkBufferImageCopy range{};
        range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.imageSubresource.baseArrayLayer = 0;
        range.imageSubresource.layerCount = 1;
        range.imageExtent.width = _image->width;
        range.imageExtent.height = _image->height;
        ez_update_image(_texture, range, _image->data.data());

        barrier = ez_image_barrier(_texture, EZ_RESOURCE_STATE_SHADER_RESOURCE | EZ_RESOURCE_STATE_UNORDERED_ACCESS);
        ez_pipeline_barrier(0, 0, nullptr, 1, &barrier);
    }
}