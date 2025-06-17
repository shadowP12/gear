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

void CLight::serialize(SerializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.object([&]() {
        ctx.field("class_name", get_class_name());
        ctx.field("type", _type);
        ctx.field("intensity", _intensity);
        ctx.field("color", _color);
        ctx.field("range", _range);
        ctx.field("spot_angle", _spot_angle);
        ctx.field("inner_angle", _inner_angle);
        ctx.field("has_shadow", _has_shadow);
    });
}

void CLight::deserialize(DeserializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.field("type", _type);
    ctx.field("intensity", _intensity);
    ctx.field("color", _color);
    ctx.field("range", _range);
    ctx.field("spot_angle", _spot_angle);
    ctx.field("inner_angle", _inner_angle);
    ctx.field("has_shadow", _has_shadow);
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

void CLight::set_spot_angle(float spot_angle)
{
    make_render_dirty();
    _spot_angle = spot_angle;
}

void CLight::set_inner_angle(float inner_angle)
{
    make_render_dirty();
    _inner_angle = inner_angle;
}

void CLight::set_has_shadow(bool has_shadow)
{
    make_render_dirty();
    _has_shadow = has_shadow;
}

void CLight::predraw()
{
    glm::mat4 light_transform = TransformUtil::remove_scale(_entity->get_world_transform());
    glm::vec3 direction = _entity->get_front_vector();
    glm::vec3 pos = _entity->get_world_translation();

    if (_type == LightType::Point || _type == LightType::Spot)
    {
        PunctualLight* light_obj = nullptr;
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

        float radius = glm::max(0.001f, get_range());
        light_obj->inv_radius = 1.0f / radius;
        light_obj->color = get_color();
        light_obj->intensity = get_intensity();
        light_obj->position = pos;
        light_obj->direction = direction;
        light_obj->cone_angle = glm::radians(get_spot_angle());
        light_obj->inner_angle = glm::radians(get_inner_angle());
    }
    else if (_type == LightType::Direction)
    {
        if (_light == INVALID_OBJECT_S)
        {
            _light = g_scene->dir_lights.add();
        }

        DirectionLight* light_obj = g_scene->dir_lights.get_obj(_light);
        light_obj->color = get_color();
        light_obj->intensity = get_intensity();
        light_obj->direction = direction;
        light_obj->has_shadow = get_has_shadow();
    }
}

REGISTER_ENTITY_COMPONENT(CLight)