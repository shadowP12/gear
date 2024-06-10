#version 450
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) out vec4 out_gbuffer_0;
layout(location = 1) out vec4 out_gbuffer_1;

void main()
{
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}