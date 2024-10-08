#version 450
#extension GL_GOOGLE_include_directive : enable

#include "material.glsl"

layout(location = 0) in vec4 vertex_world_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;

layout(location = 0) out vec4 scene_color;

void main()
{
    Material material = get_material(vertex_uv);

    vec3 sun_direction = normalize(vec3(0.0, -1.0, -1.0));
    scene_color = vec4(max(dot(normalize(vertex_normal), -sun_direction), 0.0) * vec3(0.6) + vec3(0.1), 1.0);
}