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
    writer.EndObject();
}

void CLight::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    _type = LightType(value["type"].GetInt());
    _intensity = value["intensity"].GetDouble();
    _color = Serialization::r_vec3(value["color"]);
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

REGISTER_ENTITY_COMPONENT(CLight)