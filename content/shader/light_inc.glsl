#ifndef LIGHT_H
#define LIGHT_H

#define MAX_LIGHT_DATA_STRUCTS 32

struct Cluster
{
    vec4 aabb_min;
    vec4 aabb_max;
    uvec4 lit_bits;
};

struct PunctualLight
{
    vec3 position;
    float inv_radius;

    vec3 direction;
    float size;

    vec3 color;
    float intensity;

    float cone_angle;
    float inner_angle;
    float pad0;
    float pad1;
};

struct DirectionLight
{
    vec3 direction;
    float size;
    vec3 color;
    float intensity;
};

#ifdef USING_CLUSTER_LIGHTING
layout(set = 0, binding = USING_CLUSTER_LIGHTING, std430) buffer restrict ClusterBuffer
{
    Cluster data[];
} s_clusters;


layout(set = 0, binding = USING_CLUSTER_LIGHTING + 1) uniform PointLitBuffer
{
    PunctualLight data[MAX_LIGHT_DATA_STRUCTS];
} u_point_lits;

layout(set = 0, binding = USING_CLUSTER_LIGHTING + 2) uniform SpotLitBuffer
{
    PunctualLight data[MAX_LIGHT_DATA_STRUCTS];
} u_spot_lits;
#endif

float get_distance_attenuation(float distance, float inv_radius)
{
    float attenuation = clamp(1.0 - (distance * inv_radius), 0.0, 1.0);
    return attenuation * attenuation;
}

float get_angle_attenuation(vec3 L, vec3 spot_dir, float cone_outer_angle, float cone_inner_angle)
{
    float outer_angle = cone_outer_angle * 0.5;
    float inner_angle = cone_inner_angle * 0.5;

    float cos_outer = cos(outer_angle);
    float spot_scale = 1.0 / max(cos(inner_angle) - cos_outer, 1e-4);
    float spot_offset = -cos_outer * spot_scale;

    float cd = dot(normalize(spot_dir), normalize(-L));
    float attenuation = clamp(cd * spot_scale + spot_offset, 0.0, 1.0);
    return attenuation * attenuation;
}

#endif