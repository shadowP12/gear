#ifndef UNIFORMS_H
#define UNIFORMS_H

#ifdef USING_FRAME_UNIFORMS
layout(std140, binding = USING_FRAME_UNIFORMS) uniform FrameConstants
{
    uvec4 cluster_size;
    vec4 view_position;
    mat4 view_matrix;
    mat4 proj_matrix;
    mat4 inv_view_proj_matrix;
    vec2 z_near_far;
    float ev100;
    float exposure;
} u_frame;
#endif

#ifdef USING_SCENE_UNIFORMS
layout(std140, binding = USING_SCENE_UNIFORMS) uniform InstanceData
{
    mat4 transform;
} u_scene;
#endif

#endif