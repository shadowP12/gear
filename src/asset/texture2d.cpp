#include "texture2d.h"

Texture2D::Texture2D(const std::string& asset_path)
    : Asset(asset_path)
{
}

Texture2D::~Texture2D()
{
}

void Texture2D::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, std::vector<uint8_t>& bin)
{
    writer.StartObject();
    writer.Key("uri");
    writer.String(_uri.c_str());
    writer.EndObject();
}

void Texture2D::deserialize(const rapidjson::Value& reader, const std::vector<uint8_t>& bin)
{
    _uri = reader["uri"].GetString();
}