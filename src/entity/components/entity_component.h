#pragma once

#include <core/module.h>
#include <core/serializable.h>
#include <map>
#include <functional>

class Entity;
class EntityComponent : public Serializable
{
public:
    EntityComponent(Entity*);

    virtual ~EntityComponent();

    static std::string get_static_class_name() { return "EntityComponent"; }
    virtual std::string get_class_name() { return "EntityComponent"; }

    bool is_dirty() { return _dirty; }
    void make_dirty();
    void dirty_notify();
    virtual void dirty_notify_imp() {}

protected:
    bool _dirty = false;
    Entity* _entity;
};

class EntityComponentFactory : public Module<EntityComponentFactory>
{
public:
    EntityComponentFactory() = default;
    ~EntityComponentFactory() = default;

    EntityComponent* create(Entity* entity, const std::string& component_name)
    {
        auto iter = _component_dict.find(component_name);
        if (iter != _component_dict.end())
        {
            return _component_dict[component_name](entity);
        }
        return nullptr;
    }

    void insert(const std::string& name, std::function<EntityComponent*(Entity*)> create_func)
    {
        _component_dict.insert({name, create_func});
    }

private:
    std::map<std::string, std::function<EntityComponent*(Entity*)>> _component_dict;
};

template<typename T>
class EntityComponentRegister
{
public:
    EntityComponentRegister(const std::string& name)
    {
        std::function<EntityComponent*(Entity*)> f = [](Entity* e) {
            return new T(e);
        };
        EntityComponentFactory::get()->insert(name, f);
    }

    ~EntityComponentRegister() = default;
};

#define REGISTER_ENTITY_COMPONENT(NAME) \
    static EntityComponentRegister<NAME> reg##NAME(#NAME);
