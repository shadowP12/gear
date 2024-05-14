#pragma once

#include "asset.h"

class Texture2D;

enum class MaterialAlphaMode
{
    Opaque = 0,
    Mask = 1,
    Blend = 2,
};

class Material : public Asset
{
public:
    Material(const std::string& asset_path = "");

    virtual ~Material();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);
    virtual void deserialize(const rapidjson::Value& value, Serialization::BinaryStream& bin);

    void set_base_color(glm::vec4 base_color);

    glm::vec4 get_base_color();

    void set_base_color_texture(Texture2D* base_color_texture);

    Texture2D* get_base_color_texture();

    void set_alpha_mode(MaterialAlphaMode alpha_mode);

    MaterialAlphaMode get_alpha_mode();

private:
    glm::vec4 _base_color;
    Texture2D* _base_color_texture = nullptr;
    MaterialAlphaMode _alpha_mode;
};