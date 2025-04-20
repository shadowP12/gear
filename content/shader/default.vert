#version 450
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec4 vertex_world_position;
layout(location = 1) out vec3 vertex_normal;
layout(location = 2) out vec2 vertex_uv;

#define USING_FRAME_UNIFORMS 0
#define USING_SCENE_UNIFORMS 1
#include "uniforms.glsl"

void main()
{
    vertex_uv = in_texcoord;
    vertex_normal = mat3(transpose(inverse(u_scene.transform))) * in_normal;
    vertex_world_position = u_scene.transform * vec4(in_position, 1.0);
    gl_Position = u_frame.proj_matrix * u_frame.view_matrix * vertex_world_position;
}
