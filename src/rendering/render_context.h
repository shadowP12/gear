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

    void builtin();
    void clear();
    void update();

    enum class CreateStatus
    {
        Keep,
        Recreated,
    };

    EzSampler find_samper(const std::string& name);

    UniformBuffer* find_ub(const std::string& name);
    UniformBuffer* create_ub(const std::string& name, uint32_t size, CreateStatus& status);

    TextureRef* find_texture_ref(const std::string& name);
    TextureRef* create_texture_ref(const std::string& name, const EzTextureDesc& desc, CreateStatus& status);

private:
    std::unordered_map<std::string, EzSampler> _sampler_cache;
    std::unordered_map<std::string, UniformBuffer*> _ub_cache;
    std::unordered_map<std::string, TextureRef*> _t_ref_cache;
};