#pragma once

#include "asset.h"
#include "rendering/material_constants.h"

class Texture2D;
class MaterialProxy;

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

    int get_material_id() { return _material_id; };

private:
    int _material_id;
    MaterialProxy* _proxy;
    glm::vec4 _base_color;
    Texture2D* _base_color_texture = nullptr;
    MaterialAlphaMode _alpha_mode;
};