#version 450
#extension GL_GOOGLE_include_directive : enable

#include "vertex_factory.glsl"

layout(location = 0) out vec4 vertex_world_position;
layout(location = 1) out vec3 vertex_normal;
layout(location = 2) out vec2 vertex_uv;

layout(std140, binding = 0) uniform FrameConstants
{
    mat4 view_matrix;
    mat4 proj_matrix;
} frame_constants;

layout(std140, binding = 1) uniform InstanceData
{
    mat4 transform;
} instance_data;

void main()
{
    vertex_uv = in_texcoord;
    vertex_normal = in_normal;
    vertex_world_position = instance_data.transform * vec4(in_position, 1.0);
    gl_Position = frame_constants.proj_matrix * frame_constants.view_matrix * vertex_world_position;
}
