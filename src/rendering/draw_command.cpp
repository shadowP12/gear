#include "draw_command.h"
#include "render_context.h"
#include "program.h"
#include "vertex_factory.h"
#include <algorithm>
#include <functional>

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

void DrawCommandList::draw(RenderContext* ctx)
{
    EzBuffer scene_ub = ctx->get_buffer("u_scene");
    EzBuffer frame_ub = ctx->get_buffer("u_frame");
    EzBuffer u_point_lits = ctx->get_buffer("u_point_lits");
    EzBuffer u_spot_lits = ctx->get_buffer("u_spot_lits");
    EzBuffer s_clusters = ctx->get_buffer("s_clusters");

    for (int i = 0; i < cmd_count; ++i)
    {
        DrawCommand* cmd = &cmds[i];
        VertexFactory* vertex_factory = cmd->vertex_factory;
        Program* program = cmd->program;

        program->set_parameter("screen_size", &ctx->screen_size);
        program->set_parameter("u_frame", frame_ub);
        program->set_parameter("u_scene", scene_ub, sizeof(SceneInstanceData), cmd->scene_index * sizeof(SceneInstanceData));
        program->set_parameter("u_point_lits", u_point_lits);
        program->set_parameter("u_spot_lits", u_spot_lits);
        program->set_parameter("s_clusters", s_clusters);
        program->bind();

        ez_set_primitive_topology(vertex_factory->prim_topo);
        ez_set_vertex_layout(vertex_factory->layout);
        ez_bind_vertex_buffers(0, vertex_factory->vertex_buffer_count, vertex_factory->vertex_buffers);
        ez_bind_index_buffer(vertex_factory->index_buffer, vertex_factory->index_type);

        ez_draw_indexed(vertex_factory->index_count, vertex_factory->instance_count, 0, 0, 0);
    }
}