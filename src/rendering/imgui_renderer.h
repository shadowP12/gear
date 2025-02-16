#pragma once

#include <rhi/ez_vulkan.h>
#include <map>

class Window;
class RenderContext;
class GpuBuffer;
class IndexBuffer;

class ImGuiRenderer
{
public:
    ImGuiRenderer();

    ~ImGuiRenderer();

    void render(RenderContext* ctx, Window* window);

private:
    GpuBuffer* _vertex_buffer = nullptr;
    GpuBuffer* _index_buffer = nullptr;
};