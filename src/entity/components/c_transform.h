#pragma once

#include "entity_component.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

class Entity;
class CTransform : public EntityComponent
{
public:
    CTransform(Entity* entity);

    virtual ~CTransform();

    static std::string get_static_class_name() { return "CTransform"; }
    virtual std::string get_class_name() { return "CTransform"; }

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);
    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin);

    void set_parent(Entity* new_parent);

    Entity* get_parent();

    const std::vector<Entity*>& get_children();

    void set_position(const glm::vec3& pos);

    glm::vec3 get_position() { return _translation; }

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

private:
    void update_transform();

    void update_local_transform();

private:
    glm::vec3 _translation;
    glm::vec3 _scale;
    glm::vec3 _euler;
    glm::quat _rot;
    glm::mat4 _local;
    glm::mat4 _world;
    Entity* _parent = nullptr;
    std::vector<Entity*> _children;
};