#include "draw_command.h"
#include <algorithm>
#include <functional>

float DrawLabel::WHITE[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float DrawLabel::RED[4] = {1.0f, 0.0f, 0.0f, 1.0f};

DrawLabel::DrawLabel(const char* label_name, const float color[4])
{
    ez_begin_debug_label(label_name, color);
}

DrawLabel::~DrawLabel()
{
    ez_end_debug_label();
}

void DrawCommandList::clear()
{
    cmd_count = 0;
}

DrawCommand* DrawCommandList::add_element()
{
    if (cmds.size() < cmd_count + 1)
    {
        cmds.emplace_back();
    }
    return &cmds[cmd_count++];
}

void DrawCommandList::sort()
{
    if (cmd_count <= 0)
        return;

    std::function<bool(const DrawCommand&, const DrawCommand&)> f = [](const DrawCommand& a, const DrawCommand& b)
    {
        return a.sort.sort_key < b.sort.sort_key;
    };
    std::sort(&cmds[0], &cmds[0] + cmd_count, f);
}

void DrawCommandList::sort_by_depth()
{
    if (cmd_count <= 0)
        return;

    std::function<bool(const DrawCommand&, const DrawCommand&)> f = [](const DrawCommand& a, const DrawCommand& b)
    {
        return a.distance < b.distance;
    };
    std::sort(&cmds[0], &cmds[0] + cmd_count, f);
}

