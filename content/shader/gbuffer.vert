#version 450
#extension GL_GOOGLE_include_directive : enable

#include "vertex_factory.glsl"

layout(std140, binding = 0) uniform FrameConstants
{
    mat4 view_matrix;
    mat4 proj_matrix;
} frame_constants;

layout(std140, binding = 1) uniform SceneTransform
{
    mat4 transform;
} scene_transform;

void main()
{
    gl_Position = vec4(vec3(0.0), 1.0);
}
