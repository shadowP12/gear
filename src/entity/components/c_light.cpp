#include "c_light.h"
#include "entity/entity.h"
#include "rendering/render_scene.h"

CLight::CLight(Entity* entity)
    : CRender(entity)
{
    _light = INVALID_OBJECT_S;
}

CLight::~CLight()
{
    destroy_light();
}

void CLight::destroy_light()
{
    if (_light != INVALID_OBJECT_S)
    {
        if (_type == LightType::Point)
        {
            g_scene->point_lights.remove(_light);
        }
        else if (_type == LightType::Spot)
        {
            g_scene->spot_lights.remove(_light);
        }
        else if (_type == LightType::Direction)
        {
            g_scene->dir_lights.remove(_light);
        }

        _light = INVALID_OBJECT_S;
    }
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
    destroy_light();
    make_render_dirty();
    _type = type;
}

void CLight::set_intensity(float intensity)
{
    make_render_dirty();
    _intensity = intensity;
}

void CLight::set_color(glm::vec3 color)
{
    make_render_dirty();
    _color = color;
}

void CLight::set_range(float range)
{
    make_render_dirty();
    _range = range;
}

void CLight::set_attenuation(float attenuation)
{
    make_render_dirty();
    _attenuation = attenuation;
}

void CLight::set_spot_angle(float spot_angle)
{
    make_render_dirty();
    _spot_angle = spot_angle;
}

void CLight::set_spot_attenuation(float spot_attenuation)
{
    make_render_dirty();
    _spot_attenuation = spot_attenuation;
}

void CLight::predraw()
{
    glm::mat4 light_transform = TransformUtil::remove_scale(_entity->get_world_transform());
    glm::vec3 direction = _entity->get_front_vector();
    glm::vec3 pos = _entity->get_world_translation();

    if (_type == LightType::Point || _type == LightType::Spot)
    {
        OmniLight* light_obj = nullptr;
        if (_type == LightType::Point)
        {
            if (_light == INVALID_OBJECT_S)
            {
                _light = g_scene->point_lights.add();
            }

            light_obj = g_scene->point_lights.get_obj(_light);
        }
        else
        {
            if (_light == INVALID_OBJECT_S)
            {
                _light = g_scene->spot_lights.add();
            }

            light_obj = g_scene->spot_lights.get_obj(_light);
        }

        light_obj->color = get_color();
        light_obj->intensity = get_intensity();

        float radius = glm::max(0.001f, get_range());
        light_obj->inv_radius = 1.0f / radius;

        light_obj->position = pos;
        light_obj->direction = direction;

        light_obj->attenuation = get_attenuation();
        light_obj->cone_attenuation = get_spot_attenuation();

        float spot_angle = get_spot_angle();
        light_obj->cone_angle = glm::radians(spot_angle);
    }
    else if (_type == LightType::Direction)
    {
        // TODO
    }
}

REGISTER_ENTITY_COMPONENT(CLight)