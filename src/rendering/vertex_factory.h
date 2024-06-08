#pragma once

#include <rhi/ez_vulkan.h>
#include <unordered_map>

#define STATIC_MESH_VERTEX_FACTORY 0

EzVertexBinding& get_vertex_factory_layout(int id);