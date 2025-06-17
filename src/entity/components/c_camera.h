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

    virtual void serialize(SerializationContext& ctx, BinaryStream& bin_stream);

    virtual void deserialize(DeserializationContext& ctx, BinaryStream& bin_stream);

    glm::mat4 get_proj_matrix();

    void set_orthogonal();

    void set_perspective();

    ProjectionMode get_proj_mode() { return _mode; }

    void set_near(float near);

    float get_near() { return _near; }

    void set_far(float far);

    float get_far() { return _far; }

    void set_exposure(float exposure);

    float get_exposure() { return _exposure; }

    void set_uasge(ViewUsageFlags usage);

    ViewUsageFlags get_usage() { return _usage; }

protected:
    void predraw() override;

private:
    float _exposure = 1.0f;
    float _near = 0.0f;
    float _far = 100.0f;
    float _fov = 45.0f;
    ViewUsageFlags _usage;
    ProjectionMode _mode;
};