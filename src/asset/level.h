#pragma once

#include "asset.h"

class Entity;

class Level : public Asset
{
public:
    Level(const std::string& asset_path = "");

    virtual ~Level();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);
    virtual void deserialize(const rapidjson::Value& value, Serialization::BinaryStream& bin);

protected:
    std::vector<Entity*> _entities;
};