#version 450
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(std140, binding = 0) uniform PerPass
{
    uvec2 sm_face;
} u_pass;

#if defined(Vertical)
layout(set = 0, binding = 1) uniform sampler2D t_input;
#else
layout(set = 0, binding = 1) uniform sampler2DArray t_input;
#endif

void main()
{
    vec2 size = vec2(textureSize(t_input, 0));
    vec2 texel_size = vec2(1.0) / size;

    vec4 sum = vec4(0.0);
    float kernel_size = 3.0;
    float radius = kernel_size / 2.0;
    int sample_radius = int(round(radius));
    for(int i = -sample_radius; i <= sample_radius; ++i)
    {
        #if defined(Vertical)
        vec4 s = texture(t_input, vec2(in_uv + vec2(0.0, i * texel_size.y)));
        #else
        vec4 s = texture(t_input, vec3(in_uv + vec2(i * texel_size.x, 0.0), 0.0));
        #endif
        s *= saturate((radius + 0.5f) - abs(i));
        sum += s;
    }
    out_color = sum / kernel_size;
}