#version 450

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec4 in_color;
layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2D im_texture;

void main()
{
    out_color = in_color * texture(im_texture, in_uv);
}