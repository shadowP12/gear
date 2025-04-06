#version 450
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec4 vertex_world_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;

layout(location = 0) out vec4 scene_color;

#define USING_FRAME_UNIFORMS 0
#include "uniforms.glsl"

// 2 - 4
#define USING_CLUSTER_LIGHTING 2
#include "light_inc.glsl"

layout(std140, binding = 5) uniform Params
{
    uvec2 screen_size;
} u_params;

uint get_cluster_index(vec4 frag_coord)
{
    float z_near = u_frame.z_near_far.x;
    float z_far = u_frame.z_near_far.y;

    float depth = frag_coord.z;
    depth = z_near * z_far / (z_far + depth * (z_near - z_far)); // Linearize depth
    uint z_tile = uint(log2(abs(depth) / z_near) / log2(z_far / z_near) * float(u_frame.cluster_size.z));

    vec2 tile_size = u_params.screen_size / u_frame.cluster_size.xy;
    uvec3 tile = uvec3(frag_coord.xy / tile_size, z_tile);
    uint tile_index = tile.x + (tile.y * u_frame.cluster_size.x) + (tile.z * u_frame.cluster_size.x * u_frame.cluster_size.y);
    return tile_index;
}

void main()
{
    uint cluster_index = get_cluster_index(gl_FragCoord);

    vec3 sun_direction = normalize(vec3(0.0, -1.0, -1.0));
    scene_color = vec4(max(dot(normalize(vertex_normal), -sun_direction), 0.0) * vec3(0.6) + vec3(0.1), 1.0);
}