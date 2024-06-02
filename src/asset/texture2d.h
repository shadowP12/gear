#pragma once

#include "asset.h"
#include <rhi/ez_vulkan.h>

class Image;
class Texture2D : public Asset
{
public:
    Texture2D(const std::string& asset_path = "");

    virtual ~Texture2D();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);
    virtual void deserialize(const rapidjson::Value& value, Serialization::BinaryStream& bin);

    Image* get_image() { return _image; }

    EzTexture get_texture() { return _texture; }

private:
    void generate_texture();

private:
    std::string _uri;
    Image* _image;
    EzTexture _texture;
};