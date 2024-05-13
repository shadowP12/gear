#pragma once

#include "components/entity_component.h"

class Entity : public Serializable
{
public:
    Entity();

    ~Entity();

    void set_name(const std::string& name);

    std::string get_name();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);
    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin);

    template<class T, class... Args>
    T* add_component(Args &&... args)
    {
        T* component = new T(this, std::forward<Args>(args)...);
        _components.push_back(component);
        return component;
    }

    template<class T>
    void remove_component() {
        T* component = get_component<T>();
        if (component == nullptr) {
            return;
        }

        for (auto iter = _components.begin(); iter != _components.end();) {
            if (iter == _components.end())
                break;
            if (*iter == component) {
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
        for (int i = 0; i < _components.size(); ++i) {
            if(_components[i]->get_class_name() == T::get_static_class_name()) {
                return (T*)_components[i];
            }
        }
        return nullptr;
    }

    template<class T>
    bool has_component()
    {
        for (int i = 0; i < _components.size(); ++i) {
            if(_components[i]->get_class_name() == T::get_static_class_name()) {
                return true;
            }
        }
        return false;
    }

    std::vector<EntityComponent*>& get_components() { return _components; }

protected:
    std::string _name;
    std::vector<EntityComponent*> _components;
};