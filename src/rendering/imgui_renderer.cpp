#include "imgui_renderer.h"
#include "render_context.h"
#include "window.h"

ImGuiRenderer::ImGuiRenderer()
{
}

ImGuiRenderer::~ImGuiRenderer()
{

}

void ImGuiRenderer::render(RenderContext* ctx, Window* window)
{
    ImGuiContext* imgui_ctx = window->get_imgui_ctx();
}