#include "render_resources.h"

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