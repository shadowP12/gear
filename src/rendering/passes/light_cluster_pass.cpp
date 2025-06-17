#include "light_cluster_pass.h"
#include "rendering/debug_renderer.h"
#include "rendering/program.h"
#include "rendering/render_context.h"
#include "rendering/render_scene.h"
#include "rendering/render_shared_data.h"
#include "rendering/utils/debug_utils.h"
#include "rendering/utils/render_utils.h"

#define MAX_LIGHT_DATA_STRUCTS 32

static uint32_t k_cluster_size_x = 12;
static uint32_t k_cluster_size_y = 12;
static uint32_t k_cluster_size_z = 12;
static uint32_t k_cluster_count = k_cluster_size_x * k_cluster_size_y * k_cluster_size_z;

struct Cluster {
    glm::vec4 aabb_min;
    glm::vec4 aabb_max;
    glm::uvec4 lit_bits;
};

LightClusterPass::LightClusterPass()
{
    ProgramDesc program_desc;
    program_desc.cs = "shader://cluster_debug.comp";
    _debug_program = std::make_unique<Program>(program_desc);

    program_desc.cs = "shader://cluster_aabb.comp";
    _aabb_program = std::make_unique<Program>(program_desc);

    program_desc.cs = "shader://clustering.comp";
    _clustering_program = std::make_unique<Program>(program_desc);
}

LightClusterPass::~LightClusterPass()
{
}

void LightClusterPass::setup(RenderContext* ctx)
{
    ctx->cluster_size = glm::uvec4(k_cluster_size_x, k_cluster_size_y, k_cluster_size_z, 0);
}

void LightClusterPass::exec(RenderContext* ctx)
{
    DebugLabel debug_label("Light cluster pass", DebugLabel::WHITE);

    // Draw lights
    /*
    for (int i = 0; i < g_scene->point_lights.size(); ++i)
    {
        const PunctualLight* point_lit = g_scene->point_lights[i];
        g_debug_renderer->add_sphere(glm::vec3(0.3, 0.3, 0.8), point_lit->position, 1.0f / point_lit->inv_radius);
    }
    for (int i = 0; i < g_scene->spot_lights.size(); ++i)
    {
        const PunctualLight* spot_lit = g_scene->spot_lights[i];
        g_debug_renderer->add_cone(glm::vec3(0.3, 0.3, 0.8), spot_lit->position, spot_lit->direction, spot_lit->cone_angle, 1.0f / spot_lit->inv_radius);
    }
    */

    uint32_t update_lit_count = 0;

    EzBufferDesc buffer_desc{};
    buffer_desc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    buffer_desc.size = MAX_LIGHT_DATA_STRUCTS * sizeof(PunctualLight);
    EzBuffer u_point_lits = ctx->create_buffer("u_point_lits", buffer_desc, false);
    update_lit_count = glm::min((uint32_t)MAX_LIGHT_DATA_STRUCTS, (uint32_t)g_scene->point_lights.size());
    update_render_buffer(u_point_lits, EZ_RESOURCE_STATE_SHADER_RESOURCE, update_lit_count * sizeof(PunctualLight), 0, (uint8_t*)g_scene->point_lights.data());

    buffer_desc.size = MAX_LIGHT_DATA_STRUCTS * sizeof(PunctualLight);
    EzBuffer u_spot_lits = ctx->create_buffer("u_spot_lits", buffer_desc, false);
    update_lit_count = glm::min((uint32_t)MAX_LIGHT_DATA_STRUCTS, (uint32_t)g_scene->spot_lights.size());
    update_render_buffer(u_spot_lits, EZ_RESOURCE_STATE_SHADER_RESOURCE, update_lit_count * sizeof(PunctualLight), 0, (uint8_t*)g_scene->spot_lights.data());

    buffer_desc.size = sizeof(DirectionLight);
    EzBuffer u_dir_lits = ctx->create_buffer("u_dir_lits", buffer_desc, false);
    update_lit_count = glm::min((uint32_t)1, (uint32_t)g_scene->dir_lights.size());
    update_render_buffer(u_dir_lits, EZ_RESOURCE_STATE_SHADER_RESOURCE, update_lit_count * sizeof(DirectionLight), 0, (uint8_t*)g_scene->dir_lights.data());

    buffer_desc.size = k_cluster_count * sizeof(Cluster);
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
    buffer_desc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    EzBuffer s_clusters = ctx->create_buffer("s_clusters", buffer_desc);

    VkBufferMemoryBarrier2 barrier;
    barrier = ez_buffer_barrier(s_clusters, EZ_RESOURCE_STATE_SHADER_RESOURCE);
    ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

    // Gen cluster aabbs
    RenderView* view = &g_scene->view[DISPLAY_VIEW];
    glm::vec2 z_near_far = glm::vec2(view->zn, view->zf);
    glm::uvec2 screen_size = ctx->screen_size;
    glm::uvec4 cluster_size = glm::uvec4(k_cluster_size_x, k_cluster_size_y, k_cluster_size_z, 0);

    _aabb_program->set_parameter("near_far", &z_near_far);
    _aabb_program->set_parameter("screen_size", &screen_size);
    _aabb_program->set_parameter("cluster_size", &cluster_size);
    _aabb_program->set_parameter("inv_proj", &view->inv_proj);
    _aabb_program->set_parameter("s_clusters", s_clusters);
    _aabb_program->bind();

    ez_dispatch(k_cluster_size_x, k_cluster_size_y, k_cluster_size_z);

    barrier = ez_buffer_barrier(s_clusters, EZ_RESOURCE_STATE_SHADER_RESOURCE);
    ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

    // Assigning lights
    glm::uvec2 lit_count = glm::uvec2(g_scene->point_lights.size(), g_scene->spot_lights.size());
    glm::mat4 view_matrix = view->view;

    _clustering_program->set_parameter("lit_count", &lit_count);
    _clustering_program->set_parameter("view_matrix", &view_matrix);
    _clustering_program->set_parameter("u_point_lits", u_point_lits);
    _clustering_program->set_parameter("u_spot_lits", u_spot_lits);
    _clustering_program->set_parameter("u_dir_lits", u_dir_lits);
    _clustering_program->set_parameter("s_clusters", s_clusters);
    _clustering_program->bind();

    ez_dispatch(k_cluster_size_x, k_cluster_size_y, k_cluster_size_z);

    barrier = ez_buffer_barrier(s_clusters, EZ_RESOURCE_STATE_SHADER_RESOURCE);
    ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);
}

void LightClusterPass::debug(RenderContext* ctx)
{
    DebugLabel debug_label("Light cluster debug pass", DebugLabel::WHITE);

    EzBuffer s_clusters = ctx->get_buffer("s_clusters");
    EzBuffer frame_ub = ctx->get_buffer("u_frame");

    EzTexture scene_color_rt = ctx->get_texture("scene_color");
    EzTexture scene_depth_rt = ctx->get_texture("scene_depth");

    VkImageMemoryBarrier2 rt_barriers[2];
    rt_barriers[0] = ez_image_barrier(scene_color_rt, EZ_RESOURCE_STATE_UNORDERED_ACCESS);
    rt_barriers[1] = ez_image_barrier(scene_depth_rt, EZ_RESOURCE_STATE_SHADER_RESOURCE);
    ez_pipeline_barrier(0, 0, nullptr, 2, rt_barriers);

    glm::uvec2 screen_size = ctx->screen_size;

    _debug_program->set_parameter("frame_ub", frame_ub);
    _debug_program->set_parameter("screen_size", &screen_size);
    _debug_program->set_parameter("s_clusters", s_clusters);
    _debug_program->set_parameter("screen_buffer", scene_color_rt);
    _debug_program->set_parameter("depth_buffer", scene_depth_rt, g_rsd->get_sampler(SamplerType::NearestClamp));

    _debug_program->bind();

    ez_dispatch(std::max(1u, (uint32_t)(screen_size.x) / 8), std::max(1u, (uint32_t)(screen_size.y) / 8), 1);

    rt_barriers[0] = ez_image_barrier(scene_color_rt, EZ_RESOURCE_STATE_RENDERTARGET);
    rt_barriers[1] = ez_image_barrier(scene_depth_rt, EZ_RESOURCE_STATE_DEPTH_WRITE);
    ez_pipeline_barrier(0, 0, nullptr, 2, rt_barriers);
}