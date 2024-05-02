#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() :
    SceneNode()
{
}

glm::mat4 Camera::get_proj_matrix()
{
    if (_proj_dirty)
    {
        _proj_matrix = glm::perspective(_fov, _aspect, _near, _far);
        // reverse y axis
        _proj_matrix[1][1] *= -1;
        _proj_dirty = false;
    }
    return _proj_matrix;
}

glm::mat4 Camera::get_view_matrix()
{
    _update_transform();
    return _view_matrix;
}

void Camera::_update_transform_imp()
{
    _view_matrix = glm::inverse(_transform);
}

void Camera::set_fov(float fov)
{
    _fov = fov;
    _proj_dirty = true;
}

void Camera::set_near_far(float near, float far)
{
    _near = near;
    _far = far;
    _proj_dirty = true;
}

void Camera::set_aspect(float aspect)
{
    _aspect = aspect;
    _proj_dirty = true;
}