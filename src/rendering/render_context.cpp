#include "render_context.h"

RenderContext::RenderContext()
{}

RenderContext::~RenderContext()
{
    clear();
}

void RenderContext::clear()
{
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

void RenderContext::update(float dt)
{
    delta_time = dt;
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

UniformBuffer* RenderContext::find_or_create_ub(const std::string& name, uint32_t size)
{
    bool is_new;
    return find_or_create_ub(name, size, is_new);
}

UniformBuffer* RenderContext::find_or_create_ub(const std::string& name, uint32_t size, bool& is_new)
{
    is_new = false;
    UniformBuffer* ub = find_ub(name);
    if (ub == nullptr || ub->get_buffer()->size != size)
    {
        delete ub;
        ub = new UniformBuffer(size);
        _ub_cache[name] = ub;
        is_new = true;
    }
    return ub;
}

TextureRef* RenderContext::find_t_ref(const std::string& name)
{
    auto iter = _t_ref_cache.find(name);
    if (iter != _t_ref_cache.end())
    {
        return iter->second;
    }
    return nullptr;
}

TextureRef* RenderContext::find_or_create_t_ref(const std::string& name, const EzTextureDesc& desc)
{
    bool is_new;
    return find_or_create_t_ref(name, desc, is_new);
}

TextureRef* RenderContext::find_or_create_t_ref(const std::string& name, const EzTextureDesc& desc, bool& is_new)
{
    is_new = false;
    TextureRef* t_ref = find_t_ref(name);
    if (t_ref == nullptr || compute_texture_ref_hash(t_ref->get_desc()) != compute_texture_ref_hash(desc))
    {
        delete t_ref;
        t_ref = new TextureRef(desc);
        _t_ref_cache[name] = t_ref;
        is_new = true;
    }
    return t_ref;
}