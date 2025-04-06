#include "post_process_pass.h"
#include "rendering/program.h"
#include "rendering/render_context.h"
#include "rendering/render_shared_data.h"
#include "rendering/utils/debug_utils.h"
#include "rendering/utils/render_utils.h"

PostProcessPass::PostProcessPass()
{
    _tonemapping_program = std::make_unique<Program>("shader://post_process/tonemapping.vert", "shader://post_process/tonemapping.frag");
}

PostProcessPass::~PostProcessPass()
{}

void PostProcessPass::exec(RenderContext* ctx)
{
    DebugLabel debug_label("Post process pass", DebugLabel::WHITE);

    EzTexture scene_color_rt = ctx->get_texture("scene_color");

    RenderContext::CreateStatus create_status;
    EzTextureDesc texture_desc{};
    texture_desc.width = scene_color_rt->width;
    texture_desc.height = scene_color_rt->height;
    texture_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    texture_desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    EzTexture post_process_rt_1 = ctx->create_texture("post_process_1", texture_desc, create_status);
    if (create_status == RenderContext::CreateStatus::Recreated)
    {
        ez_create_texture_view(post_process_rt_1, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    }

    EzTexture post_process_rt_2 = ctx->create_texture("post_process_2", texture_desc, create_status);
    if (create_status == RenderContext::CreateStatus::Recreated)
    {
        ez_create_texture_view(post_process_rt_2, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    }

    ez_reset_pipeline_state();
    EzBlendState blend_state;
    ez_set_blend_state(blend_state);

    EzDepthState depth_state;
    depth_state.depth_test = false;
    depth_state.depth_write = false;
    ez_set_depth_state(depth_state);

    uint32_t rt_width = scene_color_rt->width;
    uint32_t rt_height = scene_color_rt->height;
    glm::vec4 vp = glm::vec4(0, 0, rt_width, rt_height);

    // Tonemapping & Gamma correction
    {
        DebugLabel debug_label_1("Tone-Mapping", DebugLabel::WHITE);

        _swap_output = false;
        VkImageMemoryBarrier2 rt_barriers[2];
        rt_barriers[0] = ez_image_barrier(scene_color_rt, EZ_RESOURCE_STATE_SHADER_RESOURCE);
        rt_barriers[1] = ez_image_barrier(post_process_rt_1, EZ_RESOURCE_STATE_RENDERTARGET);
        ez_pipeline_barrier(0, 0, nullptr, 2, rt_barriers);

        EzRenderingInfo rendering_info{};
        EzRenderingAttachmentInfo color_info{};
        color_info.texture = post_process_rt_1;
        color_info.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
        color_info.clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
        rendering_info.colors.push_back(color_info);
        rendering_info.width = rt_width;
        rendering_info.height = rt_height;

        ez_begin_rendering(rendering_info);
        ez_set_viewport(vp.x, vp.y, vp.z, vp.w);
        ez_set_scissor((int32_t)vp.x, (int32_t)vp.y, (int32_t)vp.z, (int32_t)vp.w);

        _tonemapping_program->set_parameter("input_texture", scene_color_rt, g_rsd->get_sampler(SamplerType::LinearClamp));
        _tonemapping_program->bind();

        ez_set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        ez_set_vertex_binding(0, 20);
        ez_set_vertex_attrib(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
        ez_set_vertex_attrib(0, 1, VK_FORMAT_R32G32_SFLOAT, 12);
        ez_bind_vertex_buffer(0, g_rsd->quad_buffer);
        ez_draw(6, 0);

        ez_end_rendering();
    }

    auto get_output_in = [&]()
    {
        return _swap_output ? post_process_rt_1 : post_process_rt_2;
    };

    auto get_output_out = [&]()
    {
        return _swap_output ? post_process_rt_2 : post_process_rt_1;
    };
}

EzTexture PostProcessPass::get_final_rt(RenderContext* ctx)
{
    return _swap_output ? ctx->get_texture("post_process_2") : ctx->get_texture("post_process_1");
}