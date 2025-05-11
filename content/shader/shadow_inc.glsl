#ifndef SHADOW_INC_H
#define SHADOW_INC_H

#define SHADOW_CASCADE_COUNT 2

struct PerShadowInfo
{
    mat4 light_matrixs[6];
    vec4 cascade_splits;
};

#ifdef USING_SHADOW_INFO
layout(set = 0, binding = USING_SHADOW_INFO) uniform ShadowInfoBuffer
{
    PerShadowInfo data[1];
} u_shadow_infos;
#endif

#ifdef USING_SHADOW_MAP
layout(set = 0, binding = USING_SHADOW_MAP) uniform sampler2DArrayShadow t_shadow_map;

float sampler_shadow_map(uint cascade, vec3 shadow_position, float bias)
{
    vec2 uv = shadow_position.xy;
    float depth = shadow_position.z - bias;
    return texture(t_shadow_map, vec4(uv, cascade, saturate(depth)));
}
#endif

#endif