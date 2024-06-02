#include "mesh_utilities.h"

namespace MeshUtilities
{
EzBuffer create_mesh_buffer(void* data, uint32_t data_size, VkBufferUsageFlags usage)
{
    EzBuffer buffer;
    EzBufferDesc buffer_desc = {};
    buffer_desc.size = data_size;
    buffer_desc.usage = usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_desc.memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    ez_create_buffer(buffer_desc, buffer);

    VkBufferMemoryBarrier2 barrier;
    if (data)
    {
        barrier = ez_buffer_barrier(buffer, EZ_RESOURCE_STATE_COPY_DEST);
        ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);
        ez_update_buffer(buffer, data_size, 0, data);
    }

    EzResourceState flag = EZ_RESOURCE_STATE_UNDEFINED;
    if ((usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) != 0)
        flag |= EZ_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if ((usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) != 0)
        flag |= EZ_RESOURCE_STATE_INDEX_BUFFER;
    if ((usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) != 0)
        flag |= EZ_RESOURCE_STATE_INDIRECT_ARGUMENT;
    barrier = ez_buffer_barrier(buffer, EZ_RESOURCE_STATE_SHADER_RESOURCE | EZ_RESOURCE_STATE_UNORDERED_ACCESS | flag);
    ez_pipeline_barrier(0, 1, &barrier, 0, nullptr);

    return buffer;
}
}