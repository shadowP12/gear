#pragma once

#include "c_render.h"
#include "rendering/light.h"

class Entity;

class CLight : public CRender
{
public:
    CLight(Entity* entity);

    virtual ~CLight();

    static std::string get_static_class_name() { return "CLight"; }

    virtual std::string get_class_name() { return "CLight"; }

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);

    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin);

    void set_light_type(LightType type);

    LightType get_light_type() { return _type; }

    void set_intensity(float intensity);

    float get_intensity() { return _intensity; }

    void set_color(glm::vec3 color);

    glm::vec3 get_color() { return _color; }

    void set_range(float range);

    float get_range() { return _range; }

    void set_spot_angle(float spot_angle);

    float get_spot_angle() { return _spot_angle; }

    void set_inner_angle(float inner_angle);

    float get_inner_angle() { return _inner_angle; }

protected:
    void destroy_light();

    void predraw() override;

private:
    LightType _type;
    uint32_t _light;
    glm::vec3 _color;
    float _intensity = 0.0f;
    float _range = 5.0f;
    float _spot_angle = 45.0f;
    float _inner_angle = 40.0f;
};