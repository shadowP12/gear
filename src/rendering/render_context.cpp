#include "render_context.h"
#include <window.h>
#include <core/hash.h>

std::size_t compute_texture_hash(const EzTextureDesc& desc)
{
    std::size_t hash = 0;
    hash_combine(hash, desc.width);
    hash_combine(hash, desc.height);
    hash_combine(hash, desc.depth);
    hash_combine(hash, desc.levels);
    hash_combine(hash, desc.layers);
    hash_combine(hash, desc.format);
    hash_combine(hash, desc.image_type);
    hash_combine(hash, desc.usage);
    hash_combine(hash, desc.samples);
    hash_combine(hash, desc.memory_flags);
    return hash;
}

RenderContext::RenderContext()
{
}

RenderContext::~RenderContext()
{
    for (auto& iter : _buffer_cache)
    {
        ez_destroy_buffer(iter.second);
    }
    _buffer_cache.clear();

    for (auto& iter : _texture_cache)
    {
        ez_destroy_texture(iter.second);
    }
    _texture_cache.clear();
}

void RenderContext::collect_info(Window* window)
{
    viewport_size = window->get_size();
    screen_size =  glm::uvec2(viewport_size.z, viewport_size.w);
}

EzBuffer RenderContext::get_buffer(const std::string& name)
{
    auto iter = _buffer_cache.find(name);
    if (iter != _buffer_cache.end())
    {
        return iter->second;
    }
    return nullptr;
}

EzBuffer RenderContext::create_buffer(const std::string& name, const EzBufferDesc& desc, bool fit)
{
    EzBuffer buffer = get_buffer(name);
    if (buffer == nullptr || buffer->size != desc.size)
    {
        if (buffer)
        {
            if ( !fit && buffer->size > desc.size )
            {
                return buffer;
            }

            ez_destroy_buffer(buffer);
        }

        ez_create_buffer(desc, buffer);
        _buffer_cache[name] = buffer;
    }
    return buffer;
}

EzTexture RenderContext::get_texture(const std::string& name)
{
    auto iter = _texture_cache.find(name);
    if (iter != _texture_cache.end())
    {
        return iter->second;
    }
    return nullptr;
}

EzTexture RenderContext::create_texture(const std::string& name, const EzTextureDesc& desc, CreateStatus& status)
{
    status = CreateStatus::Keep;
    EzTexture texture = get_texture(name);
    if (texture == nullptr || compute_texture_hash(_texture_desc_cache[name]) != compute_texture_hash(desc))
    {
        if (texture)
        {
            ez_destroy_texture(texture);
        }

        ez_create_texture(desc, texture);
        _texture_cache[name] = texture;
        _texture_desc_cache[name] = desc;
        status = CreateStatus::Recreated;
    }
    return texture;
}