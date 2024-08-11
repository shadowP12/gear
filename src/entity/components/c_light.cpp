#include "c_light.h"
#include "entity/entity.h"
#include "entity/entity_notifications.h"

CLight::CLight(Entity* entity)
    : EntityComponent(entity)
{
}

CLight::~CLight()
{
}

void CLight::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("class_name");
    writer.String(get_class_name().c_str());
    writer.Key("type");
    writer.Int(int(_type));
    writer.Key("intensity");
    writer.Double(_intensity);
    writer.Key("color");
    Serialization::w_vec3(writer, _color);
    writer.Key("range");
    writer.Double(_range);
    writer.Key("attenuation");
    writer.Double(_attenuation);
    writer.Key("spot_angle");
    writer.Double(_spot_angle);
    writer.Key("spot_attenuation");
    writer.Double(_spot_attenuation);
    writer.EndObject();
}

void CLight::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    _type = LightType(value["type"].GetInt());
    _intensity = value["intensity"].GetDouble();
    _color = Serialization::r_vec3(value["color"]);
    _range = value["range"].GetDouble();
    _attenuation = value["attenuation"].GetDouble();
    _spot_angle = value["spot_angle"].GetDouble();
    _spot_attenuation = value["spot_attenuation"].GetDouble();
}

void CLight::set_light_type(LightType type)
{
    _type = type;
}

void CLight::set_intensity(float intensity)
{
    _intensity = intensity;
}

void CLight::set_color(glm::vec3 color)
{
    _color = color;
}

void CLight::set_range(float range)
{
    _range = range;
}

void CLight::set_attenuation(float attenuation)
{
    _attenuation = attenuation;
}

void CLight::set_spot_angle(float spot_angle)
{
    _spot_angle = spot_angle;
}

void CLight::set_spot_attenuation(float spot_attenuation)
{
    _spot_attenuation = spot_attenuation;
}

REGISTER_ENTITY_COMPONENT(CLight)