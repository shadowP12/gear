#include "level.h"
#include "entity/entity.h"
#include "entity/components/c_transform.h"

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
        if (_entities[i]->has_component<CTransform>())
        {
            Entity* parent = _entities[i]->get_component<CTransform>()->get_parent();
            if (parent && parent->get_id() >= 0)
            {
                hierarchy[i] = parent->get_id();
            }
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
        entity->set_id(i);
        entity->deserialize(entity_value, bin);
        _entities.push_back(entity);
    }

    for(int i = 0; i < value["hierarchy"].Size(); i++)
    {
        Entity* entity = _entities[i];
        int parent_idx = value["hierarchy"][i].GetInt();
        if (parent_idx >= 0)
        {
            Entity* parent = _entities[parent_idx];
            if (entity->has_component<CTransform>())
            {
                entity->get_component<CTransform>()->set_parent(parent);
            }
        }
    }
}