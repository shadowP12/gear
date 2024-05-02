#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class SceneNode
{
public:
    SceneNode();

    glm::mat4 get_transform();

    void set_translation(const glm::vec3& translation);

    glm::vec3 get_translation(){ return _translation; }

    void set_scale(const glm::vec3& scale);

    glm::vec3 get_scale(){ return _scale; }

    void set_euler(const glm::vec3& euler);

    glm::vec3 get_euler() { return _euler; }

protected:
    void _update_transform();

    virtual void _update_transform_imp();

protected:
    bool _transform_dirty = true;
    glm::vec3 _translation;
    glm::vec3 _scale;
    glm::vec3 _euler;
    glm::mat4 _transform;
};