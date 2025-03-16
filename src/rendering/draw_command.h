#pragma once

#include "render_constants.h"
#include <rhi/ez_vulkan.h>
#include <vector>

class Program;
class VertexFactory;
class RenderContext;

struct DrawCommand
{
    uint32_t scene_index;
    float distance;
    Program* program;
    VertexFactory* vertex_factory;
    union
    {
        struct
        {
            uint64_t sort_key;
        };
    } sort;
};

struct DrawCommandList
{
    void draw(RenderContext* ctx);

    void sort();

    void sort_by_depth();

    void clear();

    DrawCommand* add_element();

    std::size_t cmd_count = 0;
    std::vector<DrawCommand> cmds;
};