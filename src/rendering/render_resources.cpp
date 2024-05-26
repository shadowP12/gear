#include "render_resources.h"
#include <core/hash.h>

UniformBuffer::UniformBuffer(uint32_t size)
{
    EzBufferDesc buffer_desc{};
    buffer_desc.size = size;
    buffer_desc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_desc.memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    ez_create_buffer(buffer_desc, _buffer);

    // Persistent mapping
    ez_map_memory(_buffer, size, 0, (void**)&_mapdata);
}

UniformBuffer::~UniformBuffer()
{
    ez_unmap_memory(_buffer);
    ez_destroy_buffer(_buffer);
}

void UniformBuffer::write(uint8_t* data, uint32_t size, uint32_t offset)
{
    memcpy(_mapdata + offset, data, size);
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