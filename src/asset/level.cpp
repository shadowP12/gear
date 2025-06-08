#include "level.h"
#include "entity/entity.h"
#include "entity/components/c_camera.h"
#include <core/uuid.h>

Level::Level(const std::string& asset_path)
    : Asset(asset_path)
{
}

Level::~Level()
{
    for (int i = 0; i < _entities.size(); ++i)
    {
        delete _entities[i];
        _entities[i] = nullptr;
    }
    _entities.clear();
}

void Level::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    std::vector<int> hierarchy;
    hierarchy.resize(_entities.size());

    writer.StartObject();
    writer.Key("entities");
    writer.StartArray();

    for (int i = 0; i < _entities.size(); ++i)
    {
        hierarchy[i] = -1;
        Entity* parent = _entities[i]->get_parent();
        if (parent && parent->get_level_id() >= 0)
        {
            hierarchy[i] = parent->get_level_id();
        }
        _entities[i]->serialize(writer, bin);
    }

    writer.EndArray();
    writer.Key("hierarchy");
    writer.StartArray();
    for (int i = 0; i < hierarchy.size(); ++i)
    {
        writer.Int(hierarchy[i]);
    }
    writer.EndArray();
    writer.EndObject();
}

void Level::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    for(int i = 0; i < value["entities"].Size(); i++)
    {
        rapidjson::Value& entity_value = value["entities"][i];
        Entity* entity = new Entity();
        entity->set_level_id(i);
        entity->deserialize(entity_value, bin);
        _entities.push_back(entity);
    }

    if (value.HasMember("hierarchy"))
    {
        for(int i = 0; i < value["hierarchy"].Size(); i++)
        {
            int child_idx = value["hierarchy"][i][0].GetInt();
            int parent_idx = value["hierarchy"][i][1].GetInt();
            Entity* entity = _entities[child_idx];
            Entity* parent = _entities[parent_idx];
            entity->set_parent(parent);
        }
    }
}