#include "imgui_renderer.h"
#include "draw_command.h"
#include "render_context.h"
#include "render_shared_data.h"
#include "render_system.h"
#include "utils/debug_utils.h"
#include "utils/render_utils.h"
#include "window.h"
#include <core/memory.h>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/matrix.hpp>
#include <rhi/rhi_shader_mgr.h>

ImGuiRenderer::ImGuiRenderer()
{
    ProgramDesc program_desc;
    program_desc.vs = "shader://imgui.vert";
    program_desc.fs = "shader://imgui.frag";
    _program = std::make_unique<Program>(program_desc);
}

ImGuiRenderer::~ImGuiRenderer()
{
}

void ImGuiRenderer::render(RenderContext* ctx, Window* window)
{
    DebugLabel debug_label("ImGui", DebugLabel::WHITE);

    ImGuiContext* imgui_ctx = window->get_imgui_ctx();
    ImGuiViewportP* imgui_viewport = imgui_ctx->Viewports[0];
    if (!imgui_viewport->DrawDataP.Valid)
    {
        return;
    }

    ImDrawData* imgui_draw_data = &imgui_viewport->DrawDataP;
    if (imgui_draw_data->TotalVtxCount <= 0)
    {
        return;
    }

    size_t vertex_size = imgui_draw_data->TotalVtxCount * sizeof(ImDrawVert);
    size_t index_size = imgui_draw_data->TotalIdxCount * sizeof(ImDrawIdx);

    EzBufferDesc buffer_desc;
    buffer_desc.size = vertex_size;
    buffer_desc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    EzBuffer vertex_buffer = ctx->create_buffer("imgui_vb", buffer_desc, false);

    buffer_desc.size = index_size;
    buffer_desc.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    EzBuffer index_buffer = ctx->create_buffer("imgui_ib", buffer_desc, false);

    uint8_t* vertex_mapdata;
    uint8_t* index_mapdata;
    ez_map_memory(vertex_buffer, (void**)&vertex_mapdata);
    ez_map_memory(index_buffer, (void**)&index_mapdata);

    uint32_t vtx_buffer_offset = 0;
    uint32_t idx_buffer_offset = 0;
    for (int i = 0; i < imgui_draw_data->CmdListsCount; i++)
    {
        const ImDrawList* cmd_list = imgui_draw_data->CmdLists[i];
        memcpy(vertex_mapdata + vtx_buffer_offset, (uint8_t*)cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(index_mapdata + idx_buffer_offset, (uint8_t*)cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtx_buffer_offset += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
        idx_buffer_offset += cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
    }
    ez_unmap_memory(vertex_buffer);
    ez_unmap_memory(index_buffer);

    EzTexture font_texture = window->get_font_texture();
    EzSampler font_sampler = g_rsd->get_sampler(SamplerType::LinearClamp);

    glm::vec4 vp = ctx->viewport_size;
    glm::mat4 proj_matrix = glm::ortho(0.0f, (float)vp.z, 0.0f, (float)vp.w, 0.0f, 10.0f);

    _program->set_parameter("proj_matrix", &proj_matrix);
    _program->set_parameter("im_texture", font_texture, font_sampler);

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

    _program->bind();

    ez_set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    ez_set_vertex_binding(0, 20);
    ez_set_vertex_attrib(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
    ez_set_vertex_attrib(0, 1, VK_FORMAT_R32G32_SFLOAT, 8);
    ez_set_vertex_attrib(0, 2, VK_FORMAT_R8G8B8A8_UNORM, 16);

    ez_bind_vertex_buffer(0, vertex_buffer);
    ez_bind_index_buffer(index_buffer, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

    uint32_t vtx_offset = 0;
    uint32_t idx_offset = 0;
    for (int i = 0; i < imgui_draw_data->CmdListsCount; i++)
    {
        const ImDrawList* cmd_list = imgui_draw_data->CmdLists[i];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            ImVec4 clip_rect = pcmd->ClipRect;

            if (clip_rect.x < vp.z && clip_rect.y < vp.w && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
            {
                if (clip_rect.x < 0.0f)
                    clip_rect.x = 0.0f;
                if (clip_rect.y < 0.0f)
                    clip_rect.y = 0.0f;

                ez_set_scissor((int32_t)clip_rect.x, (int32_t)clip_rect.y, (int32_t)clip_rect.z, (int32_t)clip_rect.w);

                ez_draw_indexed(pcmd->ElemCount, pcmd->IdxOffset + idx_offset, (int32_t)(pcmd->VtxOffset + vtx_offset));
            }
        }

        vtx_offset += cmd_list->VtxBuffer.Size;
        idx_offset += cmd_list->IdxBuffer.Size;
    }

    ez_end_rendering();
}

ImGuiRenderer* g_imgui_renderer = nullptr;