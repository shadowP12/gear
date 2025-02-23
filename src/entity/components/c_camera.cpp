#include "c_camera.h"
#include "world.h"
#include "viewport.h"
#include "entity/entity.h"
#include "entity/entity_notifications.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

CCamera::CCamera(Entity* entity)
    : CRender(entity)
{
}

CCamera::~CCamera()
{
}

void CCamera::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("class_name");
    writer.String(get_class_name().c_str());
    writer.Key("aperture");
    writer.Double(_aperture);
    writer.Key("shutter_speed");
    writer.Double(_shutter_speed);
    writer.Key("sensitivity");
    writer.Double(_sensitivity);
    writer.Key("near");
    writer.Double(_near);
    writer.Key("far");
    writer.Double(_far);
    writer.Key("fov");
    writer.Double(_fov);
    writer.Key("usage");
    writer.Int(int(_usage));
    writer.Key("mode");
    writer.Int(int(_mode));
    writer.EndObject();
}

void CCamera::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    _aperture = value["aperture"].GetDouble();
    _shutter_speed = value["shutter_speed"].GetDouble();
    _sensitivity = value["sensitivity"].GetDouble();
    _near = value["near"].GetDouble();
    _far = value["far"].GetDouble();
    _fov = value["fov"].GetDouble();
    _usage = CameraUsage(value["usage"].GetInt());
    _mode = ProjectionMode(value["mode"].GetInt());
}

glm::mat4 CCamera::get_proj_matrix()
{
    World* world = _entity->get_world();
    if (!world)
        return glm::mat4(1.0f);

    Viewport* vp = world->get_viewport();
    if (!vp)
        return glm::mat4(1.0f);

    float aspect = vp->get_aspect();
    if (_mode == ProjectionMode::PERSPECTIVE)
    {
        glm::mat4 proj_matrix;
        proj_matrix = glm::perspective(glm::radians(_fov), aspect, _near, _far);
        // Reverse y
        proj_matrix[1][1] *= -1;
        return proj_matrix;
    }
    else if (_mode == ProjectionMode::ORTHO)
    {
        glm::mat4 proj_matrix;
        float size = 1.0;
        proj_matrix = glm::ortho(-size / 2, +size / 2, -size / aspect / 2, +size / aspect / 2, _near, _far);
        return proj_matrix;
    }
    return glm::mat4(1.0f);
}

void CCamera::set_perspective()
{
    _mode = ProjectionMode::PERSPECTIVE;
}

void CCamera::set_orthogonal()
{
    _mode = ProjectionMode::ORTHO;
}

void CCamera::set_near(float near)
{
    _near = near;
}

void CCamera::set_far(float far)
{
    _far = far;
}

void CCamera::set_uasge(CameraUsage usage)
{
    _usage = usage;
}

void CCamera::set_aperture(float aperture)
{
    _aperture = aperture;
}

void CCamera::set_shutter_speed(float shutter_speed)
{
    _shutter_speed = shutter_speed;
}

void CCamera::set_sensitivity(float sensitivity)
{
    _sensitivity = sensitivity;
}

REGISTER_ENTITY_COMPONENT(CCamera)