#pragma once

#include <vector>

enum DrawCommandType
{
    DRAW_CMD_OPAQUE = 0,
    DRAW_CMD_ALPHA = 1,
    DRAW_CMD_MAX = 2,
};

struct DrawCommand
{
    int renderable = -1;
    float distance = 0.0f;
    union
    {
        struct
        {
            uint64_t vertex_factory_id : 4;
            uint64_t material_id : 16;
        };
        struct
        {
            uint64_t sort_key;
        };
    } sort;
};

struct DrawCommandList
{
    std::size_t cmd_count = 0;
    std::vector<DrawCommand> cmds;

    void sort();
    void sort_by_depth();
    void clear();
    DrawCommand* add_element();
};