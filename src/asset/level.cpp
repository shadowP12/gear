#include "level.h"

Level::Level(const std::string& asset_path)
    : Asset(asset_path)
{
}

Level::~Level()
{
}

void Level::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
}

void Level::deserialize(const rapidjson::Value& value, Serialization::BinaryStream& bin)
{
}