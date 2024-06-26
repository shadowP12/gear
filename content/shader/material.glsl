#ifndef MATERIAL_H
#define MATERIAL_H

#include "samplers_inc.glsl"

#ifdef USING_BASE_COLOR_TEXTURE
layout(binding = 10) uniform texture2D base_color_texture;
#endif

layout(std140, binding = 2) uniform MaterialParams
{
    vec4 base_color;
} material_params;

struct Material
{
    vec4 base_color;
};

Material get_material(vec2 uv)
{
    Material material;
    #ifdef USING_BASE_COLOR_TEXTURE
    material.base_color = texture(sampler2D(base_color_texture, SAMPLER_LINEAR_CLAMP), uv);
    #else
    material.base_color = material_params.base_color;
    #endif
    return material;
}

#endif