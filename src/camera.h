#pragma once
#include "scene_node.h"

class Camera : public SceneNode
{
public:
    Camera();

    glm::mat4 get_proj_matrix();

    glm::mat4 get_view_matrix();

    void set_fov(float fov);

    void set_near_far(float near, float far);

    float get_near() const { return _near; }

    float get_far() const { return _far; }

    void set_aspect(float aspect);

protected:
    void _update_transform_imp() override;

protected:
    bool _proj_dirty = true;
    float _fov = 45.0f;
    float _near = 0.1f;
    float _far = 100.0f;
    float _aspect = 1.0f;
    glm::mat4 _proj_matrix;
    glm::mat4 _view_matrix;
};

