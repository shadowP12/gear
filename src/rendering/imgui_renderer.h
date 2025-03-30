#pragma once

#include <rhi/ez_vulkan.h>
#include <map>
#include <memory>
#include "program.h"

class Window;
class RenderContext;
class Program;

class ImGuiRenderer
{
public:
    ImGuiRenderer();

    ~ImGuiRenderer();

    void render(RenderContext* ctx, Window* window);

private:
    std::unique_ptr<Program> _program;
};

extern ImGuiRenderer* g_imgui_renderer;