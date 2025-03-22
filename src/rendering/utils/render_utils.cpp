#include "render_utils.h"

EzBuffer create_render_buffer(const EzBufferDesc& desc, EzResourceState dst_state, void* data)
{
    EzBuffer buffer;
    ez_create_buffer(desc, buffer);

    VkBufferMemoryBarrier2 barrier;
    barrier = ez_buffer_barrier(buffer, EZ_RESOURCE_STATE_COPY_DEST);
    ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

    if (data)
    {
        ez_update_buffer(buffer, desc.size, 0, data);
    }

    barrier = ez_buffer_barrier(buffer, dst_state);
    ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);
    return buffer;
}

void clear_render_buffer(EzBuffer buffer, EzResourceState dst_state)
{
    if (buffer->memory_usage == VMA_MEMORY_USAGE_CPU_ONLY)
    {
        uint8_t* mapdata;
        ez_map_memory(buffer, (void**)&mapdata);
        memset(mapdata, 0, buffer->size);
        ez_unmap_memory(buffer);
    }
    else
    {
        VkBufferMemoryBarrier2 barrier;
        barrier = ez_buffer_barrier(buffer, EZ_RESOURCE_STATE_COPY_DEST);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

        ez_clear_buffer(buffer, buffer->size, 0);

        barrier = ez_buffer_barrier(buffer, dst_state);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);
    }
}

void update_render_buffer(EzBuffer buffer, EzResourceState dst_state, uint32_t size, uint32_t offset, void* data)
{
    if (buffer->memory_usage == VMA_MEMORY_USAGE_CPU_ONLY)
    {
        uint8_t* mapdata;
        ez_map_memory(buffer, (void**)&mapdata);
        memcpy(mapdata + offset, data, size);
        ez_unmap_memory(buffer);
    }
    else
    {
        VkBufferMemoryBarrier2 barrier;
        barrier = ez_buffer_barrier(buffer, EZ_RESOURCE_STATE_COPY_DEST);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

        ez_update_buffer(buffer, size, offset, data);

        barrier = ez_buffer_barrier(buffer, dst_state);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);
    }
}