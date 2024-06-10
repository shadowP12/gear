#ifndef VERTEX_FACTORY_H
#define VERTEX_FACTORY_H

#ifdef STATIC_MESH_VERTEX_FACTORY
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;
#endif

#endif