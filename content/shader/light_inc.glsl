#ifndef LIGHT_H
#define LIGHT_H

#define MAX_LIGHT_DATA_STRUCTS 32

struct Cluster
{
    vec4 aabb_min;
    vec4 aabb_max;
    uvec4 lit_bits;
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

#ifdef USING_CLUSTER_LIGHTING
layout(set = 0, binding = USING_CLUSTER_LIGHTING, std430) buffer restrict ClusterBuffer
{
    Cluster data[];
} s_clusters;


layout(set = 0, binding = USING_CLUSTER_LIGHTING + 1) uniform PointLitBuffer
{
    OmniLight data[MAX_LIGHT_DATA_STRUCTS];
} u_point_lits;

layout(set = 0, binding = USING_CLUSTER_LIGHTING + 2) uniform SpotLitBuffer
{
    OmniLight data[MAX_LIGHT_DATA_STRUCTS];
} u_spot_lits;
#endif

#endif