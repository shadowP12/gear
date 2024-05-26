#pragma once

#include "render_constants.h"
#include "render_resources.h"
#include <string>
#include <unordered_map>

class RenderContext
{
public:
    RenderContext();
    ~RenderContext();

    void update(float dt);

    UniformBuffer* find_ub(const std::string& name);
    UniformBuffer* find_ub(const std::string& name, uint32_t size);
    UniformBuffer* find_ub(const std::string& name, uint32_t size, bool& is_new);

    TextureRef* find_t_ref(const std::string& name);
    TextureRef* find_t_ref(const std::string& name, const EzTextureDesc& desc);
    TextureRef* find_t_ref(const std::string& name, const EzTextureDesc& desc, bool& is_new);

public:
    float delta_time;

private:
    std::unordered_map<std::string, UniformBuffer*> _ub_cache;
    std::unordered_map<std::string, TextureRef*> _t_ref_cache;
};