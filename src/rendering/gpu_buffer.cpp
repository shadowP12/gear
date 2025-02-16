#include "gpu_buffer.h"

GpuBuffer::GpuBuffer(BufferUsageFlags flags, uint32_t size, uint8_t* data)
{
    _cpu_access = false;
    if (enum_has_flags(flags, BufferUsageFlags::Static))
    {
        _cpu_access = false;
    }
    if (enum_has_flags(flags, BufferUsageFlags::Dynamic))
    {
        _cpu_access = true;
    }

    VkBufferUsageFlags usage = {};
    _dst_state = EZ_RESOURCE_STATE_UNDEFINED;
    if (enum_has_flags(flags, BufferUsageFlags::Vertex))
    {
        usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        _dst_state |= EZ_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    }
    if (enum_has_flags(flags, BufferUsageFlags::Index))
    {
        usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        _dst_state |= EZ_RESOURCE_STATE_INDEX_BUFFER;
    }
    if (enum_has_flags(flags, BufferUsageFlags::Uniform))
    {
        usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        _dst_state |= EZ_RESOURCE_STATE_SHADER_RESOURCE;
    }
    if (enum_has_flags(flags, BufferUsageFlags::Storage))
    {
        usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        _dst_state |= EZ_RESOURCE_STATE_SHADER_RESOURCE | EZ_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    EzBufferDesc buffer_desc{};
    buffer_desc.size = size;
    buffer_desc.usage = usage;
    buffer_desc.memory_usage = _cpu_access ? VMA_MEMORY_USAGE_CPU_ONLY : VMA_MEMORY_USAGE_GPU_ONLY;
    ez_create_buffer(buffer_desc, _buffer);

    if (_cpu_access)
        ez_map_memory(_buffer, (void**)&_mapdata);

    if (data)
        write(data, size);
}

GpuBuffer::~GpuBuffer()
{
    if (_cpu_access)
    {
        ez_unmap_memory(_buffer);
        _mapdata = nullptr;
    }
    ez_destroy_buffer(_buffer);
}

void GpuBuffer::clear(uint32_t size, uint32_t offset)
{
    if (_cpu_access)
    {
        memset(_mapdata + offset, 0, size);
    }
    else
    {
        VkBufferMemoryBarrier2 barrier;
        barrier = ez_buffer_barrier(_buffer, EZ_RESOURCE_STATE_COPY_DEST);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

        ez_clear_buffer(_buffer, size, offset);

        barrier = ez_buffer_barrier(_buffer, _dst_state);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);
    }
}

void GpuBuffer::write(uint8_t* data, uint32_t size, uint32_t offset)
{
    if (_cpu_access)
    {
        memcpy(_mapdata + offset, data, size);
    }
    else
    {
        VkBufferMemoryBarrier2 barrier;
        barrier = ez_buffer_barrier(_buffer, EZ_RESOURCE_STATE_COPY_DEST);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

        ez_update_buffer(_buffer, size, offset, data);

        barrier = ez_buffer_barrier(_buffer, _dst_state);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);
    }
}