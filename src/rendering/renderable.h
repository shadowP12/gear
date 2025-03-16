#pragma once

#include "render_constants.h"
#include <math/bounding_box.h>
#include <rhi/ez_vulkan.h>

class Program;
class VertexFactory;

struct Renderable
{
    DrawType draw_type;
    Program* program;
    VertexFactory* vertex_factory;
    glm::mat4 transform;
    BoundingBox bounding_box;
};