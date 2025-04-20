#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../colorspace.glsl"

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D input_texture;

vec3 reinhard(vec3 hdr, float k)
{
    return hdr / (hdr + k);
}

void main()
{
    vec4 color = texture(input_texture, in_uv);
    color.rgb = reinhard(color.rgb, 1.0);
    color.rgb = linear_to_srgb(color.rgb);
    out_color = color;
}