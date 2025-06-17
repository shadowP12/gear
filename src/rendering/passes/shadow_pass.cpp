#include "shadow_pass.h"
#include "rendering/features.h"
#include "rendering/program.h"
#include "rendering/render_context.h"
#include "rendering/render_scene.h"
#include "rendering/render_shared_data.h"
#include "rendering/renderable.h"
#include "rendering/utils/debug_utils.h"
#include "rendering/utils/render_utils.h"
#include "rendering/vertex_factory.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <math/transform_util.h>

uint32_t k_dimension = 2048;

ShadowPass::ShadowPass()
{
    ProgramDesc program_desc;
    program_desc.vs = "shader://quad.vert";
    program_desc.fs = "shader://vsm_convert.frag";
    _vsm_convert_program = std::make_unique<Program>(program_desc);

    program_desc.vs = "shader://quad.vert";
    program_desc.fs = "shader://vsm_blur.frag";
    _vsm_blur_horizontal_program = std::make_unique<Program>(program_desc);

    program_desc.vs = "shader://quad.vert";
    program_desc.fs = "shader://vsm_blur.frag";
    program_desc.macros.push_back("Vertical");
    _vsm_blur_vertical_program = std::make_unique<Program>(program_desc);
}

ShadowPass::~ShadowPass()
{
}

void ShadowPass::setup(RenderContext* ctx)
{
    ctx->shadow_cascade_count = SHADOW_CASCADE_COUNT;
    _draw_list.clear();

    _shadow_infos.clear();
    for (int i = 0; i < g_scene->dir_lights.size(); ++i)
    {
        DirectionLight* dir_lit = g_scene->dir_lights[i];
        if (dir_lit->has_shadow)
        {
            dir_lit->shadow = _shadow_infos.size();
            _shadow_infos.emplace_back();
        }
    }
}

void ShadowPass::process(RenderContext* ctx, Renderable* renderable)
{
    DrawCommand* draw_cmd = _draw_list.add_element();
    draw_cmd->scene_index = renderable->scene_index;
    draw_cmd->vertex_factory = renderable->vertex_factory;
    draw_cmd->program = renderable->program;
}

void ShadowPass::exec(RenderContext* ctx)
{
    RenderContext::CreateStatus create_status;
    EzTextureDesc texture_desc{};
    texture_desc.width = k_dimension;
    texture_desc.height = k_dimension;
    texture_desc.layers = SHADOW_CASCADE_COUNT;
    texture_desc.image_type = VK_IMAGE_TYPE_2D;
    texture_desc.format = VK_FORMAT_D24_UNORM_S8_UINT;
    texture_desc.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    EzTexture t_shadow_map = ctx->create_texture("t_shadow_map", texture_desc, create_status);
    if (create_status == RenderContext::CreateStatus::Recreated)
    {
        ez_create_texture_view(t_shadow_map, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, SHADOW_CASCADE_COUNT);
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
        {
            ez_create_texture_view(t_shadow_map, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, i, 1);
        }
    }

    VkImageMemoryBarrier2 rt_barriers[1];
    rt_barriers[0] = ez_image_barrier(t_shadow_map, EZ_RESOURCE_STATE_DEPTH_WRITE);
    ez_pipeline_barrier(0, 0, nullptr, 1, rt_barriers);

    EzBufferDesc buffer_desc{};
    buffer_desc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    buffer_desc.size = glm::max(1, (int)_shadow_infos.size()) * sizeof(PerShadowInfo);
    EzBuffer u_shadow_infos = ctx->create_buffer("u_shadow_infos", buffer_desc, false);

    bool has_shadow = g_scene->dir_lights.size() > 0 && g_scene->dir_lights[0]->has_shadow;
    if (has_shadow)
    {
        DirectionLight* sun = g_scene->dir_lights[0];
        uint32_t shadow_index = sun->shadow;
        RenderView* render_view = &g_scene->view[DISPLAY_VIEW];
        float camera_near = render_view->zn;
        float camera_far = render_view->zf;
        float camera_range = camera_far - camera_near;
        float min_distance = camera_near;
        float max_distance = camera_near + camera_range;
        float cascade_splits[SHADOW_CASCADE_COUNT];
        float pssm_factor = 0.5f;
        float range = max_distance - min_distance;
        float ratio = max_distance / min_distance;
        float log_ratio = glm::clamp(1.0f - pssm_factor, 0.0f, 1.0f);
        _shadow_infos[shadow_index].cascade_splits = glm::vec4(max_distance, max_distance, max_distance, max_distance);
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
        {
            float distribute = static_cast<float>(i + 1) / SHADOW_CASCADE_COUNT;
            float log_z = static_cast<float>(min_distance * powf(ratio, distribute));
            float uniform_z = min_distance + range * distribute;
            cascade_splits[i] = glm::mix(uniform_z, log_z, log_ratio);
            cascade_splits[i] = (cascade_splits[i] - camera_near) / camera_range;
            _shadow_infos[shadow_index].cascade_splits[i] = cascade_splits[i] * camera_range + camera_near;
        }

        float cs_near;
        float cs_far;
        float cascade_split = 0.0f;
        glm::mat4 camera_view_matrix = render_view->view;
        glm::mat4 camera_proj_matrix = render_view->proj;
        glm::mat4 camera_vp_matrix = camera_proj_matrix * camera_view_matrix;
        glm::mat4 camera_inv_vp_matrix = glm::inverse(camera_vp_matrix);
        glm::mat4 light_view_matrixs[SHADOW_CASCADE_COUNT];
        glm::mat4 light_projection_matrixs[SHADOW_CASCADE_COUNT];
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
        {
            cs_near = cascade_split;
            cs_far = cascade_splits[i];
            cascade_split = cs_far;

            glm::vec3 cs_view_frustum_corners[8] = {
                {-1, -1, -1.0},
                {1, -1, -1.0},
                {-1, 1, -1.0},
                {1, 1, -1.0},
                {-1, -1, 1.0},
                {1, -1, 1.0},
                {-1, 1, 1.0},
                {1, 1, 1.0},
            };

            glm::vec3 ws_view_frustum_vertices[8];
            for (uint32_t j = 0; j < 8; j++)
            {
                ws_view_frustum_vertices[j] = TransformUtil::transform_point(cs_view_frustum_corners[j], camera_inv_vp_matrix);
            }

            for (uint32_t j = 0; j < 4; j++)
            {
                glm::vec3 dist = ws_view_frustum_vertices[j + 4] - ws_view_frustum_vertices[j];
                ws_view_frustum_vertices[j + 4] = ws_view_frustum_vertices[j] + (dist * cs_near);
                ws_view_frustum_vertices[j] = ws_view_frustum_vertices[j] + (dist * cs_far);
            }

            // Get frustum center
            int vertex_count = 8;
            glm::vec3 center = glm::vec3(0.0f);
            for (uint32_t j = 0; j < vertex_count; j++)
            {
                center += ws_view_frustum_vertices[j];
            }
            center /= 8.0f;

            float radius = 0.0f;
            for (uint32_t j = 0; j < vertex_count; j++)
            {
                float distance = glm::length(ws_view_frustum_vertices[j] - center);
                radius = glm::max(radius, distance);
            }
            radius = std::ceil(radius * 16.0f) / 16.0f;

            // Snap to texel grid
            float world_units_per_texel = radius * 2.0f / (float)k_dimension;
            glm::mat4 shadow_view_matrix = glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0) + sun->direction, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 shadow_space_origin = TransformUtil::transform_point(center, shadow_view_matrix);
            glm::vec2 snap_offset(fmod(shadow_space_origin.x, world_units_per_texel), fmod(shadow_space_origin.y, world_units_per_texel));
            shadow_space_origin.x -= snap_offset.x;
            shadow_space_origin.y -= snap_offset.y;
            center = TransformUtil::transform_point(shadow_space_origin, glm::inverse(shadow_view_matrix));

            glm::vec3 max_extents = glm::vec3(radius);
            glm::vec3 min_extents = -max_extents;
            light_view_matrixs[i] = glm::lookAt(center - sun->direction * -min_extents.z, center, glm::vec3(0.0f, 1.0f, 0.0f));
            light_projection_matrixs[i] = glm::ortho(min_extents.x, max_extents.x, min_extents.y, max_extents.y, 0.0f, max_extents.z - min_extents.z);
            _shadow_infos[shadow_index].light_matrixs[i] = light_projection_matrixs[i] * light_view_matrixs[i];
        }

        // Render shadow map
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
        {
            std::string label_text = "Render cascade shadow " + std::to_string(i);
            DebugLabel debug_label(label_text.c_str(), DebugLabel::WHITE);

            std::string u_pass_name = "u_csm_pass_" + std::to_string(i);
            glm::uvec2 sm_face(shadow_index, i);
            buffer_desc.size = sizeof(glm::uvec2);
            EzBuffer u_pass = ctx->create_buffer(u_pass_name, buffer_desc, false);
            update_render_buffer(u_pass, EZ_RESOURCE_STATE_SHADER_RESOURCE, sizeof(glm::uvec2), 0, (uint8_t*)&sm_face);

            ez_reset_pipeline_state();

            EzRenderingInfo rendering_info{};
            EzRenderingAttachmentInfo depth_info{};
            depth_info.texture = t_shadow_map;
            depth_info.texture_view = i + 1;
            depth_info.clear_value.depthStencil = {1.0f, 1};
            rendering_info.depth.push_back(depth_info);
            rendering_info.width = k_dimension;
            rendering_info.height = k_dimension;

            ez_begin_rendering(rendering_info);
            ez_set_viewport(0, 0, (float)k_dimension, (float)k_dimension);
            ez_set_scissor(0, 0, (int32_t)k_dimension, (int32_t)k_dimension);

            EzBlendState blend_state;
            ez_set_blend_state(blend_state);

            EzDepthState depth_state;
            ez_set_depth_state(depth_state);

            ez_set_cull_mode(VK_CULL_MODE_FRONT_BIT);

            _draw_list.u_pass_name = u_pass_name;
            _draw_list.draw(ctx);

            ez_end_rendering();
        }
    }

    if (!_shadow_infos.empty())
    {
        update_render_buffer(u_shadow_infos, EZ_RESOURCE_STATE_SHADER_RESOURCE, _shadow_infos.size() * sizeof(PerShadowInfo), 0, (uint8_t*)_shadow_infos.data());
    }

    rt_barriers[0] = ez_image_barrier(t_shadow_map, EZ_RESOURCE_STATE_SHADER_RESOURCE);
    ez_pipeline_barrier(0, 0, nullptr, 1, rt_barriers);

    if (g_feature_config.shadow_mode == ShadowMode::VSM)
    {
        convert_to_vsm(ctx);
    }
}

void ShadowPass::convert_to_vsm(RenderContext* ctx)
{
    RenderContext::CreateStatus create_status;
    EzTextureDesc texture_desc{};
    texture_desc.width = k_dimension;
    texture_desc.height = k_dimension;
    texture_desc.layers = SHADOW_CASCADE_COUNT;
    texture_desc.image_type = VK_IMAGE_TYPE_2D;
    texture_desc.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    texture_desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    EzTexture t_vsm = ctx->create_texture("t_vsm", texture_desc, create_status);
    if (create_status == RenderContext::CreateStatus::Recreated)
    {
        ez_create_texture_view(t_vsm, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, SHADOW_CASCADE_COUNT);
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
        {
            ez_create_texture_view(t_vsm, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, i, 1);
        }
    }

    texture_desc.layers = 1;
    EzTexture t_temp_vsm = ctx->create_texture("t_temp_vsm", texture_desc, create_status);
    if (create_status == RenderContext::CreateStatus::Recreated)
    {
        ez_create_texture_view(t_temp_vsm, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    }

    VkImageMemoryBarrier2 rt_barriers[2];

    EzTexture t_shadow_map = ctx->get_texture("t_shadow_map");
    bool has_shadow = g_scene->dir_lights.size() > 0 && g_scene->dir_lights[0]->has_shadow;
    if (has_shadow)
    {
        DirectionLight* sun = g_scene->dir_lights[0];
        uint32_t shadow_index = sun->shadow;

        ez_reset_pipeline_state();
        EzBlendState blend_state;
        ez_set_blend_state(blend_state);

        EzDepthState depth_state;
        depth_state.depth_test = false;
        depth_state.depth_write = false;
        ez_set_depth_state(depth_state);

        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
        {
            std::string label_text = "Convert vsm " + std::to_string(i);
            DebugLabel debug_label(label_text.c_str(), DebugLabel::WHITE);
            std::string u_pass_name = "u_csm_pass_" + std::to_string(i);
            EzBuffer u_pass = ctx->get_buffer(u_pass_name);

            // Convert
            {
                rt_barriers[0] = ez_image_barrier(t_vsm, EZ_RESOURCE_STATE_RENDERTARGET);
                ez_pipeline_barrier(0, 0, nullptr, 1, rt_barriers);

                EzRenderingInfo rendering_info{};
                EzRenderingAttachmentInfo color_info{};
                color_info.texture = t_vsm;
                color_info.texture_view = i + 1;
                color_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
                color_info.clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
                rendering_info.colors.push_back(color_info);
                rendering_info.width = k_dimension;
                rendering_info.height = k_dimension;

                ez_begin_rendering(rendering_info);
                ez_set_viewport(0, 0, (float)k_dimension, (float)k_dimension);
                ez_set_scissor(0, 0, (int32_t)k_dimension, (int32_t)k_dimension);

                _vsm_convert_program->set_parameter("u_pass", u_pass);
                _vsm_convert_program->set_parameter("t_shadow_map", t_shadow_map, g_rsd->get_sampler(SamplerType::LinearClamp));
                _vsm_convert_program->bind();

                ez_set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
                ez_set_vertex_binding(0, 20);
                ez_set_vertex_attrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
                ez_set_vertex_attrib(0, 1, VK_FORMAT_R32G32_SFLOAT, 12);
                ez_bind_vertex_buffer(0, g_rsd->quad_buffer);
                ez_draw(6, 0);

                ez_end_rendering();
            }

            // Horizontal blur
            {
                rt_barriers[0] = ez_image_barrier(t_vsm, EZ_RESOURCE_STATE_SHADER_RESOURCE);
                rt_barriers[1] = ez_image_barrier(t_temp_vsm, EZ_RESOURCE_STATE_RENDERTARGET);
                ez_pipeline_barrier(0, 0, nullptr, 2, rt_barriers);

                EzRenderingInfo rendering_info{};
                EzRenderingAttachmentInfo color_info{};
                color_info.texture = t_temp_vsm;
                color_info.texture_view = 0;
                color_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
                color_info.clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
                rendering_info.colors.push_back(color_info);
                rendering_info.width = k_dimension;
                rendering_info.height = k_dimension;

                ez_begin_rendering(rendering_info);
                ez_set_viewport(0, 0, (float)k_dimension, (float)k_dimension);
                ez_set_scissor(0, 0, (int32_t)k_dimension, (int32_t)k_dimension);

                _vsm_blur_horizontal_program->set_parameter("u_pass", u_pass);
                _vsm_blur_horizontal_program->set_parameter("t_input", t_vsm, g_rsd->get_sampler(SamplerType::LinearClamp), i + 1);
                _vsm_blur_horizontal_program->bind();

                ez_set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
                ez_set_vertex_binding(0, 20);
                ez_set_vertex_attrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
                ez_set_vertex_attrib(0, 1, VK_FORMAT_R32G32_SFLOAT, 12);
                ez_bind_vertex_buffer(0, g_rsd->quad_buffer);
                ez_draw(6, 0);

                ez_end_rendering();
            }

            // Vertical blur
            {
                rt_barriers[0] = ez_image_barrier(t_vsm, EZ_RESOURCE_STATE_RENDERTARGET);
                rt_barriers[1] = ez_image_barrier(t_temp_vsm, EZ_RESOURCE_STATE_SHADER_RESOURCE);
                ez_pipeline_barrier(0, 0, nullptr, 2, rt_barriers);

                EzRenderingInfo rendering_info{};
                EzRenderingAttachmentInfo color_info{};
                color_info.texture = t_vsm;
                color_info.texture_view = i + 1;
                color_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
                color_info.clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
                rendering_info.colors.push_back(color_info);
                rendering_info.width = k_dimension;
                rendering_info.height = k_dimension;

                ez_begin_rendering(rendering_info);
                ez_set_viewport(0, 0, (float)k_dimension, (float)k_dimension);
                ez_set_scissor(0, 0, (int32_t)k_dimension, (int32_t)k_dimension);

                _vsm_blur_vertical_program->set_parameter("u_pass", u_pass);
                _vsm_blur_vertical_program->set_parameter("t_input", t_temp_vsm, g_rsd->get_sampler(SamplerType::LinearClamp));
                _vsm_blur_vertical_program->bind();

                ez_set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
                ez_set_vertex_binding(0, 20);
                ez_set_vertex_attrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
                ez_set_vertex_attrib(0, 1, VK_FORMAT_R32G32_SFLOAT, 12);
                ez_bind_vertex_buffer(0, g_rsd->quad_buffer);
                ez_draw(6, 0);

                ez_end_rendering();
            }
        }
    }

    rt_barriers[0] = ez_image_barrier(t_vsm, EZ_RESOURCE_STATE_SHADER_RESOURCE);
    ez_pipeline_barrier(0, 0, nullptr, 1, rt_barriers);
}

void ShadowDrawCommandList::draw(RenderContext* ctx)
{
    EzBuffer u_pass = ctx->get_buffer(u_pass_name);
    EzBuffer u_scene = ctx->get_buffer("u_scene");
    EzBuffer u_frame = ctx->get_buffer("u_frame");
    EzBuffer u_shadow_infos = ctx->get_buffer("u_shadow_infos");

    for (int i = 0; i < cmd_count; ++i)
    {
        DrawCommand* cmd = &cmds[i];
        VertexFactory* vertex_factory = cmd->vertex_factory;
        Program* program = cmd->program;

        program->set_parameter("u_pass", u_pass);
        program->set_parameter("u_frame", u_frame);
        program->set_parameter("u_scene", u_scene, sizeof(SceneInstanceData), cmd->scene_index * sizeof(SceneInstanceData));
        program->set_parameter("u_shadow_infos", u_shadow_infos);
        program->bind();

        ez_set_primitive_topology(vertex_factory->prim_topo);
        ez_set_vertex_layout(vertex_factory->layout);
        ez_bind_vertex_buffers(0, vertex_factory->vertex_buffer_count, vertex_factory->vertex_buffers);
        ez_bind_index_buffer(vertex_factory->index_buffer, vertex_factory->index_type);

        ez_draw_indexed(vertex_factory->index_count, vertex_factory->instance_count, 0, 0, 0);
    }
}