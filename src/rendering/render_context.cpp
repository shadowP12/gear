#include "render_context.h"

RenderContext::RenderContext()
{
    builtin();
}

RenderContext::~RenderContext()
{
    clear();
}

void RenderContext::builtin()
{
    EzSampler sampler;
    EzSamplerDesc sampler_desc{};
    sampler_desc.address_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_desc.address_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_desc.address_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    ez_create_sampler(sampler_desc, sampler);
    _sampler_cache["linear"] = sampler;

    sampler_desc.mag_filter = VK_FILTER_NEAREST;
    sampler_desc.min_filter = VK_FILTER_NEAREST;
    ez_create_sampler(sampler_desc, sampler);
    _sampler_cache["nearest"] = sampler;
}

void RenderContext::clear()
{
    for (auto& iter : _sampler_cache)
    {
        ez_destroy_sampler(iter.second);
    }
    _sampler_cache.clear();

    for (auto& iter : _ub_cache)
    {
        delete iter.second;
    }
    _ub_cache.clear();

    for (auto& iter : _t_ref_cache)
    {
        delete iter.second;
    }
    _t_ref_cache.clear();
}

void RenderContext::update()
{
}

EzSampler RenderContext::find_samper(const std::string& name)
{
    auto iter = _sampler_cache.find(name);
    if (iter != _sampler_cache.end())
    {
        return iter->second;
    }
    return VK_NULL_HANDLE;
}

UniformBuffer* RenderContext::find_ub(const std::string& name)
{
    auto iter = _ub_cache.find(name);
    if (iter != _ub_cache.end())
    {
        return iter->second;
    }
    return nullptr;
}

UniformBuffer* RenderContext::create_ub(const std::string& name, uint32_t size, CreateStatus& status)
{
    status = CreateStatus::Keep;
    UniformBuffer* ub = find_ub(name);
    if (ub == nullptr || ub->get_buffer()->size != size)
    {
        delete ub;
        ub = new UniformBuffer(size);
        _ub_cache[name] = ub;
        status = CreateStatus::Recreated;
    }
    return ub;
}

TextureRef* RenderContext::find_texture_ref(const std::string& name)
{
    auto iter = _t_ref_cache.find(name);
    if (iter != _t_ref_cache.end())
    {
        return iter->second;
    }
    return nullptr;
}

TextureRef* RenderContext::create_texture_ref(const std::string& name, const EzTextureDesc& desc, CreateStatus& status)
{
    status = CreateStatus::Keep;
    TextureRef* t_ref = find_texture_ref(name);
    if (t_ref == nullptr || compute_texture_ref_hash(t_ref->get_desc()) != compute_texture_ref_hash(desc))
    {
        delete t_ref;
        t_ref = new TextureRef(desc);
        _t_ref_cache[name] = t_ref;
        status = CreateStatus::Recreated;
    }
    return t_ref;
}