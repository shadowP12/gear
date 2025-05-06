#pragma once

#include "render_constants.h"
#include "draw_command.h"
#include "light.h"
#include <rhi/ez_vulkan.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Window;

class RenderContext
{
public:
    enum class CreateStatus
    {
        Keep,
        Recreated,
    };

public:
    RenderContext();

    ~RenderContext();

    void collect_info(Window* window);

    EzBuffer get_buffer(const std::string& name);

    EzBuffer create_buffer(const std::string& name, const EzBufferDesc& desc, bool fit = true);

    EzTexture get_texture(const std::string& name);

    EzTexture create_texture(const std::string& name, const EzTextureDesc& desc, CreateStatus& status);

public:
    glm::uvec4 viewport_size;
    glm::uvec2 screen_size;
    glm::uvec4 cluster_size;
    uint32_t shadow_cascade_count;
    DrawCommandList draw_lists[DRAW_MAXCOUNT];

private:
    std::unordered_map<std::string, EzBuffer> _buffer_cache;
    std::unordered_map<std::string, EzTexture> _texture_cache;
    std::unordered_map<std::string, EzTextureDesc> _texture_desc_cache;
};