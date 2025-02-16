#pragma once

#include "render_constants.h"
#include "render_resources.h"
#include "gpu_buffer.h"
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

    void collect_viewport_info(Window* window);

    GpuBuffer* get_ub(const std::string& name);
    GpuBuffer* create_ub(const std::string& name, uint32_t size);
    GpuBuffer* create_ub(const std::string& name, uint32_t size, CreateStatus& status);

    TextureRef* get_texture_ref(const std::string& name);
    TextureRef* create_texture_ref(const std::string& name, const EzTextureDesc& desc);
    TextureRef* create_texture_ref(const std::string& name, const EzTextureDesc& desc, CreateStatus& status);

public:
    glm::vec4 viewport_size;

private:
    std::unordered_map<std::string, GpuBuffer*> _ub_cache;
    std::unordered_map<std::string, TextureRef*> _t_ref_cache;
};