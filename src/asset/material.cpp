#include "material.h"
#include "texture2d.h"
#include "asset_manager.h"

Material::Material(const std::string& asset_path)
    : Asset(asset_path)
{
}

Material::~Material()
{
}

void Material::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("base_color");
    Serialization::w_vec4(writer, _base_color);
    if (_base_color_texture)
    {
        writer.Key("base_color_texture");
        writer.String(_base_color_texture->get_asset_path().c_str());
    }
    writer.EndObject();
}

void Material::deserialize(const rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    _base_color = Serialization::r_vec4(value["base_color"]);
    if (value.HasMember("base_color_texture"))
    {
        _base_color_texture = AssetManager::get()->load<Texture2D>(value["base_color_texture"].GetString());
    }
}

void Material::set_base_color(glm::vec4 base_color) {
    _base_color = base_color;
}

glm::vec4 Material::get_base_color() {
    return _base_color;
}

void Material::set_base_color_texture(Texture2D* base_color_texture) {
    _base_color_texture = base_color_texture;
}

Texture2D* Material::get_base_color_texture() {
    return _base_color_texture;
}
