#include "texture2d.h"
#include "image.h"
#include "image_utilities.h"
#include <core/path.h>

Texture2D::Texture2D(const std::string& asset_path)
    : Asset(asset_path)
{
}

Texture2D::~Texture2D()
{
    if (_image)
    {
        delete[] _image->data;
        delete _image;
    }

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
    _image = ImageUtilities::load_image(Path::fix_path(_uri));

    if (_image)
    {
        _texture = ImageUtilities::create_texture(_image);
        ez_create_texture_view(_texture, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    }
}