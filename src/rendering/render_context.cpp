#include "render_context.h"
#include <window.h>
#include <core/memory.h>

RenderContext::RenderContext()
{
}

RenderContext::~RenderContext()
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

void RenderContext::collect_info(Window* window)
{
    viewport_size = window->get_size();
}

GpuBuffer* RenderContext::get_ub(const std::string& name)
{
    auto iter = _ub_cache.find(name);
    if (iter != _ub_cache.end())
    {
        return iter->second;
    }
    return nullptr;
}

GpuBuffer* RenderContext::create_ub(const std::string& name, uint32_t size)
{
    CreateStatus status;
    return create_ub(name, size, status);
}

GpuBuffer* RenderContext::create_ub(const std::string& name, uint32_t size, CreateStatus& status)
{
    status = CreateStatus::Keep;
    GpuBuffer* ub = get_ub(name);
    if (ub == nullptr || ub->get_handle()->size != size)
    {
        delete ub;
        ub = new GpuBuffer(BufferUsageFlags::Dynamic | BufferUsageFlags::Uniform | BufferUsageFlags::Storage, size);
        _ub_cache[name] = ub;
        status = CreateStatus::Recreated;
    }
    return ub;
}

TextureRef* RenderContext::get_texture_ref(const std::string& name)
{
    auto iter = _t_ref_cache.find(name);
    if (iter != _t_ref_cache.end())
    {
        return iter->second;
    }
    return nullptr;
}

TextureRef* RenderContext::create_texture_ref(const std::string& name, const EzTextureDesc& desc)
{
    CreateStatus status;
    return create_texture_ref(name, desc, status);
}

TextureRef* RenderContext::create_texture_ref(const std::string& name, const EzTextureDesc& desc, CreateStatus& status)
{
    status = CreateStatus::Keep;
    TextureRef* t_ref = get_texture_ref(name);
    if (t_ref == nullptr || compute_texture_ref_hash(t_ref->get_desc()) != compute_texture_ref_hash(desc))
    {
        delete t_ref;
        t_ref = new TextureRef(desc);
        _t_ref_cache[name] = t_ref;
        status = CreateStatus::Recreated;
    }
    return t_ref;
}