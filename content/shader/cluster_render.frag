#version 450

// Based on Godot Engine's implementation.
// <https://github.com/godotengine/godot/tree/master>

layout(location = 0) in float depth_interp;
layout(location = 1) in flat uint element_index;

layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 1, std140) uniform State
{
    mat4 projection;
    float inv_z_far;
    uint screen_to_clusters_shift;
    uint cluster_screen_width;
    uint cluster_data_size;
    uint cluster_depth_offset;
    uint cluster_helper_offset;
    uint pad0;
    uint pad1;
} state;

struct RenderElement
{
    uint type;
    uint index;
    uint pad0;
    uint pad1;
    mat4 transform;
    vec3 scale;
    uint pad2;
};

layout(set = 0, binding = 2, std430) buffer restrict readonly RenderElements
{
    RenderElement data[];
} render_elements;

layout(set = 0, binding = 3, std430) buffer restrict ClusterRender
{
    uint data[];
} cluster_render;

void main()
{
    uvec2 cluster = uvec2(gl_FragCoord.xy) >> state.screen_to_clusters_shift;

    uint cluster_offset = cluster.x + state.cluster_screen_width * cluster.y;
    cluster_offset *= state.cluster_data_size;

    uint usage_write_offset = cluster_offset + (element_index >> 5);
    uint usage_write_bit = 1 << (element_index & uint(0x1F));

    uint aux = atomicOr(cluster_render.data[usage_write_offset], usage_write_bit);

    float unit_depth = depth_interp * state.inv_z_far;

    uint z_bit = clamp(uint(floor(unit_depth * 32.0)), 0, 31);

    uint z_write_offset = cluster_offset + state.cluster_depth_offset + element_index;
    uint z_write_bit = 1 << z_bit;

    aux = atomicOr(cluster_render.data[z_write_offset], z_write_bit);

    uint helper_write_offset = cluster_offset + state.cluster_helper_offset + element_index;
    uint z_minmax = cluster_render.data[helper_write_offset];
    uint z_min = min(z_bit, z_minmax & uint(0xFFFF));
    uint z_max = max(z_bit, z_minmax >> 16);
    z_minmax = z_min | (z_max << 16);
    cluster_render.data[helper_write_offset] = z_minmax;

    frag_color = vec4(vec3(float(aux)), 1.0);
}
