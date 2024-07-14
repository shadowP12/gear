#pragma once

#include <rhi/ez_vulkan.h>
#include <map>

class Window;
class RenderContext;
class VertexBuffer;
class IndexBuffer;

class ImGuiRenderer
{
public:
    ImGuiRenderer();

    ~ImGuiRenderer();

    void render(RenderContext* ctx, Window* window);

private:
    VertexBuffer* _vertex_buffer = nullptr;
    IndexBuffer* _index_buffer = nullptr;
};