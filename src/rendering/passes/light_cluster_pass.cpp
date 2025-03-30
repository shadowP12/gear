#include "light_cluster_pass.h"
#include "rendering/program.h"
#include "rendering/render_scene.h"
#include "rendering/render_context.h"
#include "rendering/debug_renderer.h"
#include "rendering/utils/debug_utils.h"
#include "rendering/utils/render_utils.h"

static uint32_t k_cluster_size_x = 12;
static uint32_t k_cluster_size_y = 12;
static uint32_t k_cluster_size_z = 12;
static uint32_t k_cluster_count = k_cluster_size_x * k_cluster_size_y * k_cluster_size_z;

struct Cluster
{
    glm::vec4 aabb_min;
    glm::vec4 aabb_max;
    glm::uvec2 lit_bits;
};

LightClusterPass::LightClusterPass()
{
    _aabb_program = std::make_unique<Program>("shader://cluster_aabb.comp");
    _clustering_program = std::make_unique<Program>("shader://clustering.comp");
}

LightClusterPass::~LightClusterPass()
{}

void LightClusterPass::exec(RenderContext* ctx)
{
    DebugLabel debug_label("Light cluster pass", DebugLabel::WHITE);

    // Draw lights
    /*
    for (int i = 0; i < g_scene->point_lights.size(); ++i)
    {
        const OmniLight* point_lit = g_scene->point_lights[i];
        g_debug_renderer->add_sphere(glm::vec3(0.8, 0.3, 0.3), point_lit->position, 1.0f / point_lit->inv_radius);
    }
    for (int i = 0; i < g_scene->spot_lights.size(); ++i)
    {
        const OmniLight* spot_lit = g_scene->spot_lights[i];
        g_debug_renderer->add_cone(glm::vec3(0.3, 0.3, 0.8), spot_lit->position, spot_lit->direction, spot_lit->cone_angle, 1.0f / spot_lit->inv_radius);
    }
    */

    EzBufferDesc buffer_desc{};
    buffer_desc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    buffer_desc.size = glm::max((size_t)1, g_scene->point_lights.size()) * sizeof(OmniLight);
    EzBuffer s_point_lits = ctx->create_buffer("s_point_lits", buffer_desc, false);
    update_render_buffer(s_point_lits, EZ_RESOURCE_STATE_SHADER_RESOURCE, g_scene->point_lights.size() * sizeof(OmniLight), 0, (uint8_t*)g_scene->point_lights.data());

    buffer_desc.size = glm::max((size_t)1, g_scene->spot_lights.size()) * sizeof(OmniLight);
    EzBuffer s_spot_lits = ctx->create_buffer("s_spot_lits", buffer_desc, false);
    update_render_buffer(s_spot_lits, EZ_RESOURCE_STATE_SHADER_RESOURCE, g_scene->spot_lights.size() * sizeof(OmniLight), 0, (uint8_t*)g_scene->spot_lights.data());

    buffer_desc.size = k_cluster_count * sizeof(Cluster);
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
    EzBuffer s_clusters = ctx->create_buffer("s_clusters", buffer_desc);

    VkBufferMemoryBarrier2 barrier;
    barrier = ez_buffer_barrier(s_clusters, EZ_RESOURCE_STATE_SHADER_RESOURCE);
    ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

    // Gen cluster aabbs
    RenderView* view = &g_scene->view[DISPLAY_VIEW];
    glm::vec2 z_near_far = glm::vec2(view->zn, view->zf);
    glm::vec2 screen_size = glm::vec2(ctx->viewport_size.w, ctx->viewport_size.z);
    glm::ivec4 cluster_size = glm::ivec4(k_cluster_size_x, k_cluster_size_y, k_cluster_size_z, 0);

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


    barrier = ez_buffer_barrier(s_clusters, EZ_RESOURCE_STATE_SHADER_RESOURCE);
    ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);
}

void LightClusterPass::debug(RenderContext* ctx)
{
}