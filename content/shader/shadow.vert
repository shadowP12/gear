#version 450
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(std140, binding = 0) uniform PerPass
{
    uvec2 sm_face;
} u_pass;

#define USING_SHADOW_INFO 1
#include "shadow_inc.glsl"

#define USING_FRAME_UNIFORMS 2
#define USING_SCENE_UNIFORMS 3
#include "uniforms.glsl"

void main()
{
    uint sm = u_pass.sm_face.x;
    uint face = u_pass.sm_face.y;
    vec4 vertex_world_position = u_scene.transform * vec4(in_position, 1.0);
    gl_Position = u_shadow_infos.data[sm].light_matrixs[face] * vertex_world_position;
}