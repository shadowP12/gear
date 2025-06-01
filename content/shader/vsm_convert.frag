#version 450
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(std140, binding = 0) uniform PerPass
{
    uvec2 sm_face;
} u_pass;

layout(set = 0, binding = 1) uniform sampler2DArray t_shadow_map;

vec2 compute_depth_moments_vsm(float depth)
{
    // https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps
    highp vec2 moments;
    moments.x = depth;
    moments.y = depth * depth;
    return moments;
}

void main()
{
    uint face = u_pass.sm_face.y;
    float depth = texture(t_shadow_map, vec3(in_uv, face)).r;
    depth = depth * 2.0 - 1.0;
    depth = exp(VSM_EXPONENT * depth);
    out_color.xy = compute_depth_moments_vsm(depth);
    out_color.zw = compute_depth_moments_vsm(-1.0 / depth);
}
