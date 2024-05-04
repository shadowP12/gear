#include "material.h"

Material::Material(const std::string& asset_path)
    : Asset(asset_path)
{
}

Material::~Material()
{
}

void Material::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, std::vector<uint8_t>& bin)
{
}

void Material::deserialize(const rapidjson::Value& reader, const std::vector<uint8_t>& bin)
{
}