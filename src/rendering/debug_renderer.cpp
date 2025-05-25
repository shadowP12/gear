#include "debug_renderer.h"
#include "render_context.h"
#include "render_shared_data.h"
#include "utils/debug_utils.h"
#include "utils/render_utils.h"
#include <core/memory.h>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

DebugRenderer::DebugRenderer()
{
    ProgramDesc program_desc;
    program_desc.vs = "shader://quad.vert";
    program_desc.fs = "shader://debug_geometry.frag";
    _geometry_program = std::make_unique<Program>(program_desc);
}

DebugRenderer::~DebugRenderer()
{
}

void DebugRenderer::add_cone(const glm::vec3& col, const glm::vec3& pos, const glm::vec3& dir, float angle, float h)
{
    float half_angle = angle / 2;
    float ra = 0.0f;
    float rb = glm::tan(half_angle) * h;
    glm::vec3 pa = pos;
    glm::vec3 pb = pos + dir * h;
    _cones.push_back(glm::vec4(pa, ra));
    _cones.push_back(glm::vec4(pb, rb));
    _cones.push_back(glm::vec4(col, 1.0f));
}

void DebugRenderer::add_sphere(const glm::vec3& col, const glm::vec3& pos, float r)
{
    _spheres.push_back(glm::vec4(pos, r));
    _spheres.push_back(glm::vec4(col, 1.0f));
}

void DebugRenderer::render(RenderContext* ctx)
{
    bool should_draw_geom = !_cones.empty() || !_spheres.empty();
    if (!should_draw_geom)
    {
        return;
    }

    DebugLabel debug_label("Debug renderer", DebugLabel::WHITE);

    EzBufferDesc buffer_desc{};
    buffer_desc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    buffer_desc.size = glm::max((size_t)2, _spheres.size()) * sizeof(glm::vec4);
    EzBuffer s_debug_shperes = ctx->create_buffer("s_debug_shperes", buffer_desc, false);
    update_render_buffer(s_debug_shperes, EZ_RESOURCE_STATE_SHADER_RESOURCE, _spheres.size() * sizeof(glm::vec4), 0, (uint8_t*)_spheres.data());

    buffer_desc.size = glm::max((size_t)3, _cones.size()) * sizeof(glm::vec4);
    EzBuffer s_debug_cones = ctx->create_buffer("s_spot_lits", buffer_desc, false);
    update_render_buffer(s_debug_cones, EZ_RESOURCE_STATE_SHADER_RESOURCE, _cones.size() * sizeof(glm::vec4), 0, (uint8_t*)_cones.data());

    glm::vec4 vp = ctx->viewport_size;
    EzBuffer frame_ub = ctx->get_buffer("u_frame");
    EzTexture out_color_rt = ctx->get_texture("out_color");
    uint32_t rt_width = out_color_rt->width;
    uint32_t rt_height = out_color_rt->height;

    VkImageMemoryBarrier2 rt_barriers[1];
    rt_barriers[0] = ez_image_barrier(out_color_rt, EZ_RESOURCE_STATE_RENDERTARGET);
    ez_pipeline_barrier(0, 0, nullptr, 1, rt_barriers);

    ez_reset_pipeline_state();

    EzRenderingInfo rendering_info{};
    EzRenderingAttachmentInfo color_info{};
    color_info.texture = out_color_rt;
    color_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_info.clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
    rendering_info.colors.push_back(color_info);
    rendering_info.width = rt_width;
    rendering_info.height = rt_height;

    ez_begin_rendering(rendering_info);
    ez_set_viewport(vp.x, vp.y, vp.z, vp.w);
    ez_set_scissor((int32_t)vp.x, (int32_t)vp.y, (int32_t)vp.z, (int32_t)vp.w);

    EzBlendState blend_state;
    blend_state.blend_enable = true;
    ez_set_blend_state(blend_state);

    EzDepthState depth_state;
    depth_state.depth_test = false;
    depth_state.depth_write = false;
    ez_set_depth_state(depth_state);

    if (should_draw_geom)
    {
        glm::ivec2 geom_count = glm::ivec2(_spheres.size() / 2, _cones.size() / 3);

        _geometry_program->set_parameter("geom_count", &geom_count);
        _geometry_program->set_parameter("s_spheres", s_debug_shperes);
        _geometry_program->set_parameter("s_cones", s_debug_cones);
        _geometry_program->set_parameter("u_frame", frame_ub);
        _geometry_program->bind();

        ez_set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        ez_set_vertex_binding(0, 20);
        ez_set_vertex_attrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
        ez_set_vertex_attrib(0, 1, VK_FORMAT_R32G32_SFLOAT, 12);
        ez_bind_vertex_buffer(0, g_rsd->quad_buffer);
        ez_draw(6, 0);
    }

    ez_end_rendering();

    _cones.clear();
    _spheres.clear();
}

DebugRenderer* g_debug_renderer = nullptr;