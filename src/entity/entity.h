#pragma once

#include "components/entity_component.h"
#include <core/event.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

class World;
class Entity : public Serializable
{
public:
    Entity();

    ~Entity();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);

    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin);

    void set_name(const std::string& name);

    std::string get_name();

    void set_level_id(int id);

    int get_level_id();

    void set_world(World* world);

    World* get_world();

    template<class T, class... Args>
    T* add_component(Args &&... args)
    {
        T* component = new T(this, std::forward<Args>(args)...);
        _components.push_back(component);
        return component;
    }

    template<class T>
    void remove_component()
    {
        T* component = get_component<T>();
        if (component == nullptr)
        {
            return;
        }

        for (auto iter = _components.begin(); iter != _components.end();)
        {
            if (iter == _components.end())
                break;
            if (*iter == component)
            {
                _components.erase(iter);
                delete component;
                component = nullptr;
                break;
            }
        }
    }

    template<class T>
    T* get_component()
    {
        for (int i = 0; i < _components.size(); ++i)
        {
            if(_components[i]->get_class_name() == T::get_static_class_name())
            {
                return (T*)_components[i];
            }
        }
        return nullptr;
    }

    template<class T>
    bool has_component()
    {
        for (int i = 0; i < _components.size(); ++i)
        {
            if(_components[i]->get_class_name() == T::get_static_class_name())
            {
                return true;
            }
        }
        return false;
    }

    std::vector<EntityComponent*>& get_components() { return _components; }

    void set_parent(Entity* new_parent);

    Entity* get_parent();

    const std::vector<Entity*>& get_children();

    void set_translation(const glm::vec3& pos);

    glm::vec3 get_translation() { return _translation; }

    void set_scale(const glm::vec3& scale);

    glm::vec3 get_scale() { return _scale; }

    void set_euler(const glm::vec3& euler);

    glm::vec3 get_euler() { return _euler; }

    glm::quat get_rotation() { return _rot; }

    void set_transform(const glm::mat4& local_transform);

    glm::mat4 get_transform();

    glm::mat4 get_world_transform();

    glm::vec3 get_right_vector();

    glm::vec3 get_up_vector();

    glm::vec3 get_front_vector();

    glm::vec3 get_world_translation();

protected:
    void update_transform();

    void update_local_transform();

public:
    Event<> transform_changed_event;

protected:
    int _level_id = -1;
    World* _world = nullptr;
    std::string _name;
    std::vector<EntityComponent*> _components;

    // Transform
    glm::vec3 _translation;
    glm::vec3 _scale;
    glm::vec3 _euler;
    glm::quat _rot;
    glm::mat4 _local_transform;
    glm::mat4 _world_transform;
    Entity* _parent = nullptr;
    std::vector<Entity*> _children;
};