#include "cluster_builder.h"
#include "render_system.h"
#include "render_context.h"
#include "render_shared_data.h"
#include "draw_command.h"
#include <core/bitops.h>
#include <core/memory.h>
#include <core/bitops.h>
#include <math/transform_util.h>

struct ClusterRenderConstantBlock
{
    uint32_t base_index;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
};

struct ClusterStoreConstantBlock
{
    uint32_t cluster_render_data_size;
    uint32_t cluster_helper_offset;
    uint32_t cluster_screen_size[2];
    uint32_t max_render_element_count_div_32;
    uint32_t render_element_count_div_32;
    uint32_t max_cluster_element_count_div_32;
    uint32_t pad0;
};

struct ClusterDebugConstantBlock
{
    uint32_t screen_size[2];
    uint32_t cluster_screen_size[2];
    uint32_t cluster_shift;
    uint32_t cluster_type;
    float z_near;
    float z_far;
    uint32_t orthogonal;
    uint32_t max_cluster_element_count_div_32;
    uint32_t pad0;
    uint32_t pad1;
};

ClusterBuilder::ClusterBuilder()
{
    int element_max = MAX_CLUSTER_ELEMENT * ELEMENT_TYPE_MAX;
    _elements.resize(element_max);
}

ClusterBuilder::~ClusterBuilder()
{
}

void ClusterBuilder::begin(RenderContext* ctx, RenderView* view)
{
    _screen_size.x = ctx->viewport_size.z;
    _screen_size.y = ctx->viewport_size.w;
    _cluster_screen_size.x = (_screen_size.x - 1) / MAX_CLUSTER_SIZE + 1;
    _cluster_screen_size.y = (_screen_size.y - 1) / MAX_CLUSTER_SIZE + 1;

    _zn = view->zn;
    _zf = view->zf;
    _orthogonal = view->is_orthogonal;
    _view_mat = view->view;
    _projection_mat = view->projection;
    _element_count = 0;

    _cluster_buffer_size = _cluster_screen_size.x * _cluster_screen_size.y * (MAX_CLUSTER_ELEMENT / 32 + 32) * ELEMENT_TYPE_MAX * sizeof(uint32_t);
    _max_element_count = MAX_CLUSTER_ELEMENT * ELEMENT_TYPE_MAX;
    uint32_t element_tag_bits_size = _max_element_count / 32;
    uint32_t element_tag_depth_bits_size = _max_element_count;
    uint32_t element_tag_helper_bits_size = _max_element_count;
    _cluster_render_buffer_size = _cluster_screen_size.x * _cluster_screen_size.y * (element_tag_bits_size + element_tag_depth_bits_size + element_tag_helper_bits_size) * sizeof(uint32_t);
}

void ClusterBuilder::add_light(LightType type, int index, const glm::mat4& transform, float radius, float spot_aperture)
{
    RenderSharedData* shared_data = RenderSystem::get()->get_shared_data();
    ClusterElement& element = _elements[_element_count];
    glm::mat4 transform_cs = _view_mat * transform;
    glm::vec3 light_origin = TransformUtil::get_translation(transform_cs);

    if (type == LightType::Point)
    {
        radius *= shared_data->sphere_overfit;

        element.type = ELEMENT_TYPE_POINT_LIGHT;
        element.index = index;
        element.scale = glm::vec3(radius, radius, radius);
        element.transform = transform_cs;
    }
    else // type == LightType::Spot
    {
        radius *= shared_data->cone_overfit;
        float len = glm::tan(glm::radians(spot_aperture)) * radius;

        element.type = ELEMENT_TYPE_SPOT_LIGHT;
        element.index = index;
        if (spot_aperture > WIDE_SPOT_ANGLE_THRESHOLD_DEG)
        {
            element.scale = glm::vec3(radius, radius, radius);
            element.has_wide_spot_angle = true;
        }
        else
        {
            element.scale = glm::vec3(len * shared_data->cone_overfit, len * shared_data->cone_overfit, radius);
            element.has_wide_spot_angle = false;
        }
        element.transform = transform_cs;
    }

    _element_count++;
}

void ClusterBuilder::bake(RenderContext* ctx)
{
    DrawLabel draw_label("Prepare cluster buffer", DrawLabel::WHITE);

    UniformBuffer* cluster_buffer = ctx->create_ub("cluster_buffer", _cluster_buffer_size);
    cluster_buffer->clear(_cluster_buffer_size);

    if (_element_count > 0)
    {
        UniformBuffer* cluster_render_buffer = ctx->create_ub("cluster_render_buffer", _cluster_render_buffer_size);
        cluster_render_buffer->clear(_cluster_render_buffer_size);

        // Fill state
        _state.projection = _projection_mat;
        _state.inv_z_far = 1.0f / _zf;
        _state.screen_to_clusters_shift = get_shift_from_power_of_2(MAX_CLUSTER_SIZE);
        _state.cluster_screen_width = _cluster_screen_size.x;
        _state.cluster_depth_offset = _max_element_count / 32;
        _state.cluster_helper_offset = _max_element_count / 32 + _max_element_count;
        _state.cluster_data_size = _max_element_count / 32 + _max_element_count + _max_element_count;

        UniformBuffer* cluster_state_buffer = ctx->create_ub("cluster_state_buffer", sizeof(ClusterBuilderState));
        cluster_state_buffer->write((uint8_t*)&_state, sizeof(ClusterBuilderState));

        UniformBuffer* cluster_elements_buffer = ctx->create_ub("cluster_elements_buffer", sizeof(ClusterElement) * _element_count);
        cluster_elements_buffer->write((uint8_t*)_elements.data(), sizeof(ClusterElement) * _element_count);

        // Render stage
        {
            RenderSharedData* shared_data = RenderSystem::get()->get_shared_data();

            TextureRef* out_color_ref = ctx->get_texture_ref("out_color");
            uint32_t rt_width = out_color_ref->get_desc().width;
            uint32_t rt_height = out_color_ref->get_desc().height;

            RenderContext::CreateStatus create_status;
            EzTextureDesc texture_desc{};
            texture_desc.width = rt_width;
            texture_desc.height = rt_width;
            texture_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
            texture_desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            TextureRef* cluster_rt_ref = ctx->create_texture_ref("cluster_rt", texture_desc, create_status);
            if (create_status == RenderContext::CreateStatus::Recreated)
            {
                ez_create_texture_view(cluster_rt_ref->get_texture(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
            }

            VkImageMemoryBarrier2 rt_barriers[1];
            rt_barriers[0] = ez_image_barrier(cluster_rt_ref->get_texture(), EZ_RESOURCE_STATE_RENDERTARGET);
            ez_pipeline_barrier(0, 0, nullptr, 1, rt_barriers);

            ez_reset_pipeline_state();

            EzRenderingInfo rendering_info{};
            EzRenderingAttachmentInfo color_info{};
            color_info.texture = cluster_rt_ref->get_texture();
            color_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_info.clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
            rendering_info.colors.push_back(color_info);
            rendering_info.width = rt_width;
            rendering_info.height = rt_height;

            ez_begin_rendering(rendering_info);

            glm::vec4 vp = ctx->viewport_size;
            ez_set_viewport(vp.x, vp.y, vp.z, vp.w);
            ez_set_scissor((int32_t)vp.x, (int32_t)vp.y, (int32_t)vp.z, (int32_t)vp.w);

            EzBlendState blend_state;
            blend_state.blend_enable = true;
            ez_set_blend_state(blend_state);

            EzDepthState depth_state;
            depth_state.depth_test = false;
            depth_state.depth_write = false;
            ez_set_depth_state(depth_state);

            ez_set_vertex_shader(rhi_get_shader("shader://cluster_render.vert"));
            ez_set_fragment_shader(rhi_get_shader("shader://cluster_render.frag"));

            ez_set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

            ez_set_vertex_binding(0, 12);
            ez_set_vertex_attrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);

            ez_bind_buffer(1, cluster_state_buffer->get_buffer());
            ez_bind_buffer(2, cluster_elements_buffer->get_buffer());
            ez_bind_buffer(3, cluster_render_buffer->get_buffer());

            ClusterRenderConstantBlock constant_block;
            for (int i = 0; i < _element_count; i++)
            {
                ClusterElement& element = _elements[i];
                if (element.type == ELEMENT_TYPE_POINT_LIGHT)
                {
                    constant_block.base_index = i;
                    ez_push_constants(&constant_block, sizeof(ClusterRenderConstantBlock), 0);

                    ez_bind_vertex_buffer(0, shared_data->sphere_vertex_buffer->get_buffer());
                    ez_bind_index_buffer(shared_data->sphere_index_buffer->get_buffer(), VK_INDEX_TYPE_UINT16);

                    ez_draw_indexed(80 * 3, 0, 0);
                }
                else if (element.type == ELEMENT_TYPE_SPOT_LIGHT)
                {
                    constant_block.base_index = i;
                    ez_push_constants(&constant_block, sizeof(ClusterRenderConstantBlock), 0);

                    if (element.has_wide_spot_angle)
                    {
                        ez_bind_vertex_buffer(0, shared_data->sphere_vertex_buffer ->get_buffer());
                        ez_bind_index_buffer(shared_data->sphere_index_buffer ->get_buffer(), VK_INDEX_TYPE_UINT16);

                        ez_draw_indexed(80 * 3, 0, 0);
                    }
                    else
                    {
                        ez_bind_vertex_buffer(0, shared_data->cone_vertex_buffer->get_buffer());
                        ez_bind_index_buffer(shared_data->cone_index_buffer->get_buffer(), VK_INDEX_TYPE_UINT16);

                        ez_draw_indexed(62 * 3, 0, 0);
                    }
                }
            }

            ez_end_rendering();
        }

        // Store stage
        {
            VkBufferMemoryBarrier2 barrier = ez_buffer_barrier(cluster_render_buffer->get_buffer(), EZ_RESOURCE_STATE_UNORDERED_ACCESS);
            ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

            ez_reset_pipeline_state();
            ez_set_compute_shader(rhi_get_shader("shader://cluster_store.comp"));

            ez_bind_buffer(1, cluster_render_buffer->get_buffer());
            ez_bind_buffer(2, cluster_buffer->get_buffer());
            ez_bind_buffer(3, cluster_elements_buffer->get_buffer());

            ClusterStoreConstantBlock constant_block;
            constant_block.cluster_render_data_size = _max_element_count / 32 + _max_element_count + _max_element_count;
            constant_block.cluster_helper_offset = _max_element_count / 32 + _max_element_count;
            constant_block.max_render_element_count_div_32 = _max_element_count / 32;
            constant_block.cluster_screen_size[0] = _cluster_screen_size.x;
            constant_block.cluster_screen_size[1] = _cluster_screen_size.y;
            constant_block.render_element_count_div_32 = _element_count > 0 ? (_element_count - 1) / 32 + 1 : 0;
            constant_block.max_cluster_element_count_div_32 = MAX_CLUSTER_ELEMENT / 32;
            ez_push_constants(&constant_block, sizeof(ClusterStoreConstantBlock), 0);

            ez_dispatch(std::max(1u, (uint32_t)(_cluster_screen_size.x) / 8), std::max(1u, (uint32_t)(_cluster_screen_size.y) / 8), 1);

            barrier = ez_buffer_barrier(cluster_buffer->get_buffer(), EZ_RESOURCE_STATE_UNORDERED_ACCESS);
            ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);
        }
    }
}

void ClusterBuilder::debug(RenderContext* ctx)
{
    DrawLabel draw_label("Cluster debug", DrawLabel::WHITE);

    RenderSharedData* shared_data = RenderSystem::get()->get_shared_data();

    UniformBuffer* cluster_buffer = ctx->get_ub("cluster_buffer");
    TextureRef* out_color_ref = ctx->get_texture_ref("out_color");
    TextureRef* scene_depth_ref = ctx->get_texture_ref("scene_depth");

    VkImageMemoryBarrier2 rt_barriers[2];
    rt_barriers[0] = ez_image_barrier(out_color_ref->get_texture(), EZ_RESOURCE_STATE_UNORDERED_ACCESS);
    rt_barriers[1] = ez_image_barrier(scene_depth_ref->get_texture(), EZ_RESOURCE_STATE_SHADER_RESOURCE);
    ez_pipeline_barrier(0, 0, nullptr, 2, rt_barriers);

    ez_reset_pipeline_state();
    ez_set_compute_shader(rhi_get_shader("shader://cluster_debug.comp"));

    ez_bind_buffer(1, cluster_buffer->get_buffer());
    ez_bind_texture(2, out_color_ref->get_texture(), 0);
    ez_bind_texture(3, scene_depth_ref->get_texture(), 0);
    ez_bind_sampler(4, shared_data->get_sampler(SAMPLER_NEAREST_CLAMP));

    ClusterDebugConstantBlock constant_block;
    constant_block.screen_size[0] = _screen_size.x;
    constant_block.screen_size[1] = _screen_size.y;
    constant_block.cluster_screen_size[0] = _cluster_screen_size.x;
    constant_block.cluster_screen_size[1] = _cluster_screen_size.y;
    constant_block.cluster_shift = get_shift_from_power_of_2(MAX_CLUSTER_SIZE);
    constant_block.cluster_type = ELEMENT_TYPE_SPOT_LIGHT;
    constant_block.orthogonal = _orthogonal;
    constant_block.z_far = _zf;
    constant_block.z_near = _zn;
    constant_block.max_cluster_element_count_div_32 = MAX_CLUSTER_ELEMENT / 32;

    ez_push_constants(&constant_block, sizeof(ClusterDebugConstantBlock), 0);

    ez_dispatch(std::max(1u, (uint32_t)(_screen_size.x) / 8), std::max(1u, (uint32_t)(_screen_size.y) / 8), 1);
}