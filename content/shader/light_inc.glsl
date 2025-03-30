#ifndef LIGHT_H
#define LIGHT_H

struct Cluster
{
    vec4 aabb_min;
    vec4 aabb_max;
    uvec2 lit_bits;
};

struct OmniLight
{
    vec3 position;
    float inv_radius;

    vec3 direction;
    float size;

    vec3 color;
    float intensity;

    float attenuation;
    float cone_attenuation;
    float cone_angle;
    float pad0;
};

struct DirectionLight
{
    vec3 direction;
    float size;
    vec3 color;
    float intensity;
};

#endif