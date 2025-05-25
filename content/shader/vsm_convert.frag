#version 450

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(std140, binding = 0) uniform PerPass
{
    uvec2 sm_face;
} u_pass;

layout(set = 0, binding = 1) uniform sampler2DArray t_shadow_map;

void main()
{
    uint face = u_pass.sm_face.y;
    float depth = texture(t_shadow_map, vec3(in_uv, face)).r;
    out_color = vec4(depth, depth * depth, 0.0, 0.0);
}
