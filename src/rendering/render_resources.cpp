#include "render_resources.h"
#include <core/hash.h>

UniformBuffer::UniformBuffer(uint32_t size, bool persistent)
{
    _persistent = persistent;

    EzBufferDesc buffer_desc{};
    buffer_desc.size = size;
    buffer_desc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_desc.memory_flags = persistent ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    ez_create_buffer(buffer_desc, _buffer);

    VkBufferMemoryBarrier2 barrier = ez_buffer_barrier(_buffer, EZ_RESOURCE_STATE_SHADER_RESOURCE | EZ_RESOURCE_STATE_UNORDERED_ACCESS);
    ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

    // Persistent mapping
    if (_persistent)
        ez_map_memory(_buffer, size, 0, (void**)&_mapdata);
}

UniformBuffer::~UniformBuffer()
{
    if (_persistent)
    {
        ez_unmap_memory(_buffer);
        _mapdata = nullptr;
    }
    ez_destroy_buffer(_buffer);
}

void UniformBuffer::write(uint8_t* data, uint32_t size, uint32_t offset)
{
    if (_persistent)
    {
        memcpy(_mapdata + offset, data, size);
    }
    else
    {
        VkBufferMemoryBarrier2 barrier = ez_buffer_barrier(_buffer, EZ_RESOURCE_STATE_COPY_DEST);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

        ez_update_buffer(_buffer, size, offset, data);

        barrier = ez_buffer_barrier(_buffer, EZ_RESOURCE_STATE_SHADER_RESOURCE | EZ_RESOURCE_STATE_UNORDERED_ACCESS);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);
    }
}

TextureRef::TextureRef(const EzTextureDesc& desc)
{
    _desc = desc;
    ez_create_texture(desc, _texture);
}

TextureRef::~TextureRef()
{
    ez_destroy_texture(_texture);
}

std::size_t compute_texture_ref_hash(const EzTextureDesc& desc)
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