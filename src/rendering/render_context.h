#pragma once

#include "render_constants.h"
#include "render_resources.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Window;

class RenderContext
{
public:
    RenderContext();
    ~RenderContext();

    void clear();
    void collect_viewport_info(Window* window);

    enum class CreateStatus
    {
        Keep,
        Recreated,
    };

    UniformBuffer* find_ub(const std::string& name);
    UniformBuffer* create_ub(const std::string& name, uint32_t size, CreateStatus& status);

    TextureRef* find_texture_ref(const std::string& name);
    TextureRef* create_texture_ref(const std::string& name, const EzTextureDesc& desc, CreateStatus& status);

public:
    glm::vec4 viewport_size;

private:
    std::unordered_map<std::string, UniformBuffer*> _ub_cache;
    std::unordered_map<std::string, TextureRef*> _t_ref_cache;
};