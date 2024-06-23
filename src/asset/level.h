#pragma once

#include "asset.h"
#include <core/event.h>

class Entity;

class Level : public Asset
{
public:
    Level(const std::string& asset_path = "");
    virtual ~Level();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);
    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin);

    void tick(float dt);

    Entity* get_entity(int idx) { return _entities[idx]; }

    std::vector<Entity*>& get_entities() { return _entities; }

protected:
    std::vector<Entity*> _entities;
};