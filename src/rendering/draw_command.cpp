#include "draw_command.h"
#include "render_context.h"
#include "render_shared_data.h"
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
    EzBuffer u_scene = ctx->get_buffer("u_scene");
    EzBuffer u_frame = ctx->get_buffer("u_frame");
    EzBuffer u_point_lits = ctx->get_buffer("u_point_lits");
    EzBuffer u_spot_lits = ctx->get_buffer("u_spot_lits");
    EzBuffer u_dir_lits = ctx->get_buffer("u_dir_lits");
    EzBuffer s_clusters = ctx->get_buffer("s_clusters");
    EzBuffer u_shadow_infos = ctx->get_buffer("u_shadow_infos");
    EzTexture t_shadow_map = ctx->get_texture("t_shadow_map");
    EzSampler shadow_sampler = g_rsd->get_sampler(SamplerType::Shadow);
    uint32_t shadow_map_views[] = {0, 1, 2, 3};

    for (int i = 0; i < cmd_count; ++i)
    {
        DrawCommand* cmd = &cmds[i];
        VertexFactory* vertex_factory = cmd->vertex_factory;
        Program* program = cmd->program;

        program->set_parameter("screen_size", &ctx->screen_size);
        program->set_parameter("u_frame", u_frame);
        program->set_parameter("u_scene", u_scene, sizeof(SceneInstanceData), cmd->scene_index * sizeof(SceneInstanceData));
        program->set_parameter("u_point_lits", u_point_lits);
        program->set_parameter("u_spot_lits", u_spot_lits);
        program->set_parameter("u_dir_lits", u_dir_lits);
        program->set_parameter("s_clusters", s_clusters);
        program->set_parameter("u_shadow_infos", u_shadow_infos);
        program->set_parameter("t_shadow_map", t_shadow_map, shadow_sampler, ctx->shadow_cascade_count, shadow_map_views);

        program->bind();

        ez_set_primitive_topology(vertex_factory->prim_topo);
        ez_set_vertex_layout(vertex_factory->layout);
        ez_bind_vertex_buffers(0, vertex_factory->vertex_buffer_count, vertex_factory->vertex_buffers);
        ez_bind_index_buffer(vertex_factory->index_buffer, vertex_factory->index_type);

        ez_draw_indexed(vertex_factory->index_count, vertex_factory->instance_count, 0, 0, 0);
    }
}