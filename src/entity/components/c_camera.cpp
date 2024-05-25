#include "c_camera.h"
#include "entity/entity.h"
#include "entity/entity_notifications.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

CCamera::CCamera(Entity* entity)
    : EntityComponent(entity)
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
    writer.Key("aspect");
    writer.Double(_aspect);
    writer.Key("rect");
    Serialization::w_vec4(writer, _rect);
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
    _aspect = value["aspect"].GetDouble();
    _rect = Serialization::r_vec4(value["rect"]);
    _usage = CameraUsage(value["usage"].GetInt());
    _mode = ProjectionMode(value["mode"].GetInt());

    make_dirty();
}

glm::mat4 CCamera::get_proj_matrix()
{
    glm::mat4 proj_matrix;
    if (_mode == ProjectionMode::PERSPECTIVE)
    {
        proj_matrix = glm::perspective(glm::radians(_fov), _aspect, _near, _far);
        // Reverse y
        proj_matrix[1][1] *= -1;
    }
    else if (_mode == ProjectionMode::ORTHO)
    {
        proj_matrix = glm::ortho(_rect[0], _rect[1], _rect[2], _rect[3], _near, _far);
    }
    return proj_matrix;
}

void CCamera::set_perspective()
{
    _mode = ProjectionMode::PERSPECTIVE;
    make_dirty();
}

void CCamera::set_orthogonal()
{
    _mode = ProjectionMode::ORTHO;
    make_dirty();
}

void CCamera::set_near(float near)
{
    _near = near;
    make_dirty();
}

void CCamera::set_far(float far)
{
    _far = far;
    make_dirty();
}

void CCamera::set_aspect(float aspect)
{
    _aspect = aspect;
    make_dirty();
}

void CCamera::set_rect(glm::vec4 rect)
{
    _rect = rect;
    make_dirty();
}

void CCamera::set_uasge(CameraUsage usage)
{
    _usage = usage;
    make_dirty();
}

void CCamera::set_aperture(float aperture)
{
    _aperture = aperture;
    make_dirty();
}

void CCamera::set_shutter_speed(float shutter_speed)
{
    _shutter_speed = shutter_speed;
    make_dirty();
}

void CCamera::set_sensitivity(float sensitivity)
{
    _sensitivity = sensitivity;
    make_dirty();
}

void CCamera::dirty_notify_imp()
{
    _entity->notify.broadcast(NOTIFY_CAMERA_CHANGED, _entity->get_id());
}

REGISTER_ENTITY_COMPONENT(CCamera)