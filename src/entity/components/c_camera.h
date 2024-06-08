#pragma once

#include "entity_component.h"
#include <core/enum_flag.h>

enum class ProjectionMode
{
    PERSPECTIVE = 0,
    ORTHO = 1
};

enum CameraUsage
{
    CAMERA_USAGE_NONE = 0,
    CAMERA_USAGE_MAIN = 0x1,
    CAMERA_USAGE_DISPLAY = 0x2,
    CAMERA_USAGE_MAIN_DISPLAY = CAMERA_USAGE_MAIN | CAMERA_USAGE_DISPLAY,
};
SP_MAKE_ENUM_FLAG(uint32_t, CameraUsage)

class Entity;
class CCamera : public EntityComponent
{
public:
    CCamera(Entity* entity);

    virtual ~CCamera();

    static std::string get_static_class_name() { return "CCamera"; }
    virtual std::string get_class_name() { return "CCamera"; }

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);
    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin);

    glm::mat4 get_proj_matrix();

    void set_orthogonal();

    void set_perspective();

    void set_near(float near);

    float get_near() { return _near; }

    void set_far(float far);

    float get_far() { return _far; }

    void set_aspect(float aspect);

    float get_aspect() { return _aspect; }

    void set_rect(glm::vec4 rect);

    glm::vec4 get_rect() { return _rect; }

    void set_uasge(CameraUsage usage);

    CameraUsage get_usage() { return _usage; }

    void set_aperture(float aperture);

    float get_aperture() { return _aperture; }

    void set_shutter_speed(float shutter_speed);

    float get_shutter_speed() { return _shutter_speed; }

    void set_sensitivity(float sensitivity);

    float get_sensitivity() { return _sensitivity; }

private:
    void dirty_notify_imp() override;

private:
    float _aperture = 16.0f;
    float _shutter_speed = 1.0f / 125.0f;
    float _sensitivity = 100.0f;
    float _near = 0.0f;
    float _far = 100.0f;
    float _fov = 45.0f;
    float _aspect = 1.0f;
    glm::vec4 _rect = glm::vec4(-1.0f, 1.0f, 1.0f, -1.0f); // Left, Right, Top, Bottom
    CameraUsage _usage;
    ProjectionMode _mode;
};