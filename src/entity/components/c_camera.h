#pragma once

#include "c_render.h"
#include "rendering/render_view.h"

class Entity;

class CCamera : public CRender
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

    ProjectionMode get_proj_mode() { return _mode; }

    void set_near(float near);

    float get_near() { return _near; }

    void set_far(float far);

    float get_far() { return _far; }

    void set_uasge(ViewUsageFlags usage);

    ViewUsageFlags get_usage() { return _usage; }

    void set_aperture(float aperture);

    float get_aperture() { return _aperture; }

    void set_shutter_speed(float shutter_speed);

    float get_shutter_speed() { return _shutter_speed; }

    void set_sensitivity(float sensitivity);

    float get_sensitivity() { return _sensitivity; }

protected:
    void predraw() override;

private:
    float _aperture = 16.0f;
    float _shutter_speed = 1.0f / 125.0f;
    float _sensitivity = 100.0f;
    float _near = 0.0f;
    float _far = 100.0f;
    float _fov = 45.0f;
    ViewUsageFlags _usage;
    ProjectionMode _mode;
};