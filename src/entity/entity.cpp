#include "entity.h"

Entity::Entity()
{
}

Entity::~Entity()
{
    set_parent(nullptr);
    std::vector<Entity*> remove_children = _children;
    for (int i = 0; i < remove_children.size(); ++i)
    {
        remove_children[i]->set_parent(nullptr);
    }
    _children.clear();

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

    writer.Key("translation");
    Serialization::w_vec3(writer, _translation);
    writer.Key("scale");
    Serialization::w_vec3(writer, _scale);
    writer.Key("euler");
    Serialization::w_vec3(writer, _euler);

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

    _translation = Serialization::r_vec3(value["translation"]);
    _scale = Serialization::r_vec3(value["scale"]);
    _euler = Serialization::r_vec3(value["euler"]);
    _rot = glm::quat(_euler);
    update_local_transform();
    update_transform();

    for(int i = 0; i < value["components"].Size(); i++)
    {
        rapidjson::Value& component_value = value["components"][i];
        std::string component_class_name = component_value["class_name"].GetString();
        EntityComponent* component = EntityComponentFactory::get()->create(this, component_class_name);
        component->deserialize(component_value, bin);
        _components.push_back(component);
    }
}

void Entity::set_name(const std::string& name)
{
    _name = name;
}

std::string Entity::get_name()
{
    return _name;
}

void Entity::set_id(int id)
{
    _id = id;
}

int Entity::get_id()
{
    return _id;
}

void Entity::set_level_id(int id)
{
    _level_id = id;
}

int Entity::get_level_id()
{
    return _level_id;
}

World* Entity::get_world()
{
    return _world;
}

void Entity::set_world(World* world)
{
    _world = world;
}

void Entity::set_parent(Entity* new_parent)
{
    Entity* old_parent = _parent;

    if (old_parent == new_parent)
        return;

    if (old_parent)
    {
        for (auto iter = old_parent->_children.begin(); iter != old_parent->_children.end(); iter++)
        {
            if (iter == old_parent->_children.end())
            {
                break;
            }

            if ((*iter) == this)
            {
                old_parent->_children.erase(iter);
                break;
            }
        }
    }

    _parent = new_parent;
    if (_parent)
    {
        _parent->_children.push_back(this);
    }

    update_transform();
}

Entity* Entity::get_parent()
{
    return _parent;
}

const std::vector<Entity*>& Entity::get_children()
{
    return _children;
}

void Entity::set_translation(const glm::vec3& pos)
{
    _translation = pos;
    update_local_transform();
    update_transform();
}

void Entity::set_scale(const glm::vec3& scale)
{
    _scale = scale;
    update_local_transform();
    update_transform();
}

void Entity::set_euler(const glm::vec3& euler)
{
    _euler = euler;
    _rot = glm::quat(euler);
    update_local_transform();
    update_transform();
}

void Entity::set_transform(const glm::mat4& local_transform)
{
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(local_transform, _scale, _rot, _translation, skew, perspective);
    _euler = glm::eulerAngles(_rot) * 3.14159f / 180.f;

    _local_transform = local_transform;
    update_transform();
}

glm::mat4 Entity::get_transform()
{
    return _local_transform;
}

glm::mat4 Entity::get_world_transform()
{
    return _world_transform;
}

glm::vec3 Entity::get_right_vector()
{
    return glm::vec3(_world_transform[0][0], _world_transform[0][1], _world_transform[0][2]);
}

glm::vec3 Entity::get_up_vector()
{
    return glm::vec3(_world_transform[1][0], _world_transform[1][1], _world_transform[1][2]);
}

glm::vec3 Entity::get_front_vector()
{
    return glm::vec3(_world_transform[2][0], _world_transform[2][1], _world_transform[2][2]);
}

glm::vec3 Entity::get_world_translation()
{
    return glm::vec3(_world_transform[3][0], _world_transform[3][1], _world_transform[3][2]);
}

void Entity::update_transform()
{
    _world_transform = _local_transform;
    if (_parent)
    {
        _world_transform = _parent->_world_transform * _local_transform;
    }

    for (int i = 0; i < _children.size(); ++i)
    {
        _children[i]->update_transform();
    }
}

void Entity::update_local_transform()
{
    glm::mat4 r, t, s;
    r = glm::toMat4(_rot);
    t = glm::translate(glm::mat4(1.0), _translation);
    s = glm::scale(glm::mat4(1.0), _scale);
    _local_transform = t * r * s;
}
