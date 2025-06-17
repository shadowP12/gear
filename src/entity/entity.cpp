#include "entity.h"

Entity::Entity() {}

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

void Entity::serialize(SerializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.object([&]() {
        ctx.field("name", _name);
        ctx.field("translation", _translation);
        ctx.field("scale", _scale);
        ctx.field("euler", _euler);

        ctx.array("components", [&]() {
            for (auto& component : _components)
            {
                component->serialize(ctx, bin_stream);
            }
        });
    });
}

void Entity::deserialize(DeserializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.field("name", _name);
    ctx.field("translation", _translation);
    ctx.field("scale", _scale);
    ctx.field("euler", _euler);

    _rot = glm::quat(_euler);
    update_local_transform();
    update_transform();

    ctx.array("components", [&]() {
        std::string component_class_name;
        ctx.field("class_name", component_class_name);
        EntityComponent* component = EntityComponentFactory::get()->create(this, component_class_name);
        component->deserialize(ctx, bin_stream);
        _components.push_back(component);
    });
}

void Entity::set_name(const std::string& name)
{
    _name = name;
}

std::string Entity::get_name()
{
    return _name;
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
    return glm::normalize(glm::vec3(_world_transform[0][0], _world_transform[0][1], _world_transform[0][2]));
}

glm::vec3 Entity::get_up_vector()
{
    return glm::normalize(glm::vec3(_world_transform[1][0], _world_transform[1][1], _world_transform[1][2]));
}

glm::vec3 Entity::get_front_vector()
{
    // The camera looks towards -z
    return glm::normalize(-glm::vec3(_world_transform[2][0], _world_transform[2][1], _world_transform[2][2]));
}

glm::vec3 Entity::get_world_translation()
{
    glm::vec3 ret(_world_transform[3][0], _world_transform[3][1], _world_transform[3][2]);
    return ret;
}

void Entity::update_transform()
{
    _world_transform = _local_transform;
    if (_parent)
    {
        _world_transform = _parent->_world_transform * _local_transform;
    }

    for (auto& child : _children)
    {
        child->update_transform();
    }

    transform_changed_event.broadcast();
}

void Entity::update_local_transform()
{
    glm::mat4 r, t, s;
    r = glm::toMat4(_rot);
    t = glm::translate(glm::mat4(1.0), _translation);
    s = glm::scale(glm::mat4(1.0), _scale);
    _local_transform = t * r * s;
}
