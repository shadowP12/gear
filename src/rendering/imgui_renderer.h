#pragma once

#include <rhi/ez_vulkan.h>
#include <map>
#include <memory>
#include "program.h"

class Window;
class RenderContext;
class GpuBuffer;
class Program;

class ImGuiRenderer
{
public:
    ImGuiRenderer();

    ~ImGuiRenderer();

    void render(RenderContext* ctx, Window* window);

private:
    GpuBuffer* _vertex_buffer = nullptr;
    GpuBuffer* _index_buffer = nullptr;
    std::unique_ptr<Program> _program;
};

extern ImGuiRenderer* g_imgui_renderer;