#version 450
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D input_texture;

void main()
{
    out_color = texture(input_texture, in_uv);
}