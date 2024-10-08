#version 450

// Based on Godot Engine's implementation.
// <https://github.com/godotengine/godot/tree/master>

layout(location = 0) in vec3 vertex_attrib;

layout(location = 0) out float depth_interp;
layout(location = 1) out flat uint element_index;

layout(push_constant) uniform ConstantBlock
{
    uint base_index;
    uint pad0;
    uint pad1;
    uint pad2;
} constant;

layout(set = 0, binding = 1, std140) uniform State
{
    mat4 projection;
    float inv_z_far;
    uint screen_to_clusters_shift;
    uint cluster_screen_width;
    uint cluster_data_size;
    uint cluster_depth_offset;
    uint pad0;
    uint pad1;
    uint pad2;
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

void main()
{
    element_index = constant.base_index + gl_InstanceIndex;

    vec4 vertex = vec4(vertex_attrib, 1.0);
    vertex.xyz *= render_elements.data[element_index].scale;

    vertex = render_elements.data[element_index].transform * vertex;
    depth_interp = -vertex.z;

    gl_Position = state.projection * vertex;
}
