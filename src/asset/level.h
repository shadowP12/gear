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
    std::vector<Entity*>& get_camera_entities();

public:
    Event<int, int> notify;
    void notify_received(int what, int id);

protected:
    std::vector<int> _dirty_list;
    std::vector<Entity*> _entities;
    std::vector<int> _camera_entity_indices;
};