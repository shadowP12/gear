#include "entity.h"

Entity::Entity()
{
}

Entity::~Entity()
{
    for (int i = 0; i < _components.size(); ++i)
    {
        delete _components[i];
        _components[i] = nullptr;
    }
    _components.clear();
}

void Entity::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("name");
    writer.String(_name.c_str());

    writer.Key("components");
    writer.StartArray();
    for (int i = 0; i < _components.size(); ++i)
    {
        _components[i]->serialize(writer, bin);
    }
    writer.EndArray();
    writer.EndObject();
}

void Entity::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    _name = value["name"].GetString();
    for(int i = 0; i < value["components"].Size(); i++)
    {
        rapidjson::Value& component_value = value["components"][i];
        std::string component_class_name = component_value["class_name"].GetString();
        EntityComponent* component = EntityComponentFactory::get()->create(this, component_class_name);
        component->deserialize(component_value, bin);
        _components.push_back(component);
    }
}

void Entity::set_name(const std::string& name) {
    _name = name;
}

std::string Entity::get_name() {
    return _name;
}