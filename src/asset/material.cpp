#include "material.h"
#include "texture2d.h"
#include "asset_manager.h"
#include "rendering/render_system.h"

Material::Material(const std::string& asset_path)
    : Asset(asset_path)
{
    _material_id = RenderSystem::get()->get_material_proxy_pool()->register_proxy();
    _proxy = RenderSystem::get()->get_material_proxy_pool()->get_proxy(_material_id);
}

Material::~Material()
{
    RenderSystem::get()->get_material_proxy_pool()->unregister_proxy(_material_id);
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

    writer.Key("alpha_mode");
    writer.Int((int)_alpha_mode);
    writer.EndObject();
}

void Material::deserialize(const rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    _base_color = Serialization::r_vec4(value["base_color"]);
    if (value.HasMember("base_color_texture"))
    {
        _base_color_texture = AssetManager::get()->load<Texture2D>(value["base_color_texture"].GetString());
    }
    _alpha_mode = (MaterialAlphaMode)value["alpha_mode"].GetInt();

    // Update proxy
    _proxy->params.base_color = _base_color;
    _proxy->base_color_texture = _base_color_texture ? _base_color_texture->get_texture() : VK_NULL_HANDLE;
    _proxy->alpha_mode = _alpha_mode;
    _proxy->make_dirty();
}

void Material::set_base_color(glm::vec4 base_color) {
    _base_color = base_color;
    _proxy->params.base_color = base_color;
    _proxy->make_dirty();
}

glm::vec4 Material::get_base_color() {
    return _base_color;
}

void Material::set_base_color_texture(Texture2D* base_color_texture) {
    _base_color_texture = base_color_texture;
    _proxy->base_color_texture = _base_color_texture->get_texture();
}

Texture2D* Material::get_base_color_texture() {
    return _base_color_texture;
}

void Material::set_alpha_mode(MaterialAlphaMode alpha_mode) {
    _alpha_mode = alpha_mode;
    _proxy->alpha_mode = alpha_mode;
}

MaterialAlphaMode Material::get_alpha_mode() {
    return _alpha_mode;
}