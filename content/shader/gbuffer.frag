#version 450
#extension GL_GOOGLE_include_directive : enable

#include "material.glsl"

layout(location = 0) in vec4 vertex_world_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;

layout(location = 0) out vec4 gbuffer_0;
layout(location = 1) out vec4 gbuffer_1;

void main()
{
    Material material = get_material(vertex_uv);
    gbuffer_0 = material.base_color;
    gbuffer_1 = vec4(vertex_normal, 1.0);
}