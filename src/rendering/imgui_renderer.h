#pragma once

#include <rhi/ez_vulkan.h>
#include <map>

class Window;
class RenderContext;

class ImGuiRenderer
{
public:
    ImGuiRenderer();

    ~ImGuiRenderer();

    void render(RenderContext* ctx, Window* window);

private:
    std::map<int, EzTexture> _font_textures;
};