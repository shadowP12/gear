#include "scene_node.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

SceneNode::SceneNode()
{
    _translation = glm::vec3(0.0f);
    _scale = glm::vec3(1.0f);
    _euler = glm::vec3(0.0f);
}

glm::mat4 SceneNode::get_transform()
{
    _update_transform();
    return _transform;
}

void SceneNode::_update_transform()
{
    if (_transform_dirty)
    {
        glm::mat4 r, t, s;
        r = glm::toMat4(glm::quat(_euler));
        t = glm::translate(glm::mat4(1.0), _translation);
        s = glm::scale(glm::mat4(1.0), _scale);
        _transform = t * r * s;
        _transform_dirty = false;

        _update_transform_imp();
    }
}

void SceneNode::_update_transform_imp()
{
}

void SceneNode::set_translation(const glm::vec3& translation)
{
    _translation = translation;
    _transform_dirty = true;
}

void SceneNode::set_scale(const glm::vec3& scale)
{
    _scale = scale;
    _transform_dirty = true;
}

void SceneNode::set_euler(const glm::vec3& euler)
{
    _euler = euler;
    _transform_dirty = true;
}