#include "c_transform.h"
#include "entity/entity.h"

CTransform::CTransform(Entity* entity)
    : EntityComponent(entity)
{
}

CTransform::~CTransform()
{
    set_parent(nullptr);

    std::vector<Entity*> remove_children = _children;
    for (int i = 0; i < remove_children.size(); ++i) {
        remove_children[i]->get_component<CTransform>()->set_parent(nullptr);
    }
    _children.clear();
}

void CTransform::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("class_name");
    writer.String(get_class_name().c_str());
    writer.Key("translation");
    Serialization::w_vec3(writer, _translation);
    writer.Key("scale");
    Serialization::w_vec3(writer, _scale);
    writer.Key("euler");
    Serialization::w_vec3(writer, _euler);
    writer.EndObject();
}

void CTransform::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    _translation = Serialization::r_vec3(value["translation"]);
    _scale = Serialization::r_vec3(value["scale"]);
    _euler = Serialization::r_vec3(value["euler"]);
    _rot = glm::quat(_euler);
    update_local_transform();
    update_transform();
}

void CTransform::set_parent(Entity* new_parent)
{
    Entity* old_parent = _parent;

    if (old_parent == new_parent)
        return;

    if (old_parent)
    {
        CTransform* old_parent_transform = old_parent->get_component<CTransform>();
        for (auto iter = old_parent_transform->_children.begin(); iter != old_parent_transform->_children.end(); iter++)
        {
            if (iter == old_parent_transform->_children.end()) {
                break;
            }

            if ((*iter) == this->_entity)
            {
                old_parent_transform->_children.erase(iter);
                break;
            }
        }
    }

    _parent = new_parent;
    if (_parent)
    {
        CTransform* new_parent_transform = _parent->get_component<CTransform>();
        new_parent_transform->_children.push_back(this->_entity);
    }

    update_transform();
}

Entity* CTransform::get_parent() {
    return _parent;
}

const std::vector<Entity*>& CTransform::get_children() {
    return _children;
}

void CTransform::set_position(const glm::vec3& pos)
{
    _translation = pos;
    update_local_transform();
    update_transform();
}

void CTransform::set_scale(const glm::vec3& scale)
{
    _scale = scale;
    update_local_transform();
    update_transform();
}

void CTransform::set_euler(const glm::vec3& euler)
{
    _euler = euler;
    _rot = glm::quat(euler);
    update_local_transform();
    update_transform();
}

void CTransform::set_transform(const glm::mat4& local_transform)
{
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(local_transform, _scale, _rot, _translation, skew,perspective);
    _euler = glm::eulerAngles(_rot) * 3.14159f / 180.f;

    _local = local_transform;
    update_transform();
}

glm::mat4 CTransform::get_transform() {
    return _local;
}

glm::mat4 CTransform::get_world_transform() {
    return _world;
}

glm::vec3 CTransform::get_right_vector() {
    return glm::vec3(_world[0][0], _world[0][1], _world[0][2]);
}

glm::vec3 CTransform::get_up_vector() {
    return glm::vec3(_world[1][0], _world[1][1], _world[1][2]);
}

glm::vec3 CTransform::get_front_vector() {
    return glm::vec3(_world[2][0], _world[2][1], _world[2][2]);
}

void CTransform::update_transform()
{
    _world = _local;
    if (_parent) {
        _world = _parent->get_component<CTransform>()->_world * _local;
    }

    for (int i = 0; i < _children.size(); ++i) {
        _children[i]->get_component<CTransform>()->update_transform();
    }
}

void CTransform::update_local_transform()
{
    glm::mat4 r, t, s;
    r = glm::toMat4(_rot);
    t = glm::translate(glm::mat4(1.0), _translation);
    s = glm::scale(glm::mat4(1.0), _scale);
    _local = t * r * s;
}

REGISTER_ENTITY_COMPONENT(CTransform)