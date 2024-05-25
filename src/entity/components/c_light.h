#pragma once

#include "entity_component.h"

enum class LightType
{
    POINT,
    SPOT,
    DIRECTION,
};

class Entity;
class CLight : public EntityComponent
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

private:
    void dirty_notify_imp() override;

private:
    LightType _type;
    glm::vec3 _color;
    /*
     * Direction: lux
     */
    float _intensity = 0.0f;
};