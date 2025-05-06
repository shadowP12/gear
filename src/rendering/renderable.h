#pragma once

#include "render_constants.h"
#include <math/bounding_box.h>
#include <rhi/ez_vulkan.h>

class Program;
class VertexFactory;

struct Renderable
{
    uint32_t scene_index;
    DrawType draw_type;
    Program* program;
    VertexFactory* vertex_factory;
    glm::mat4 transform;
    BoundingBox bounding_box;
    BoundingBox local_bounding_box;
};