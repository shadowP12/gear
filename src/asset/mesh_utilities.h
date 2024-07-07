#pragma once

#include <rhi/ez_vulkan.h>

namespace MeshUtilities {

EzBuffer create_mesh_buffer(void* data, uint32_t data_size, VkBufferUsageFlags usage);

} // namespace MeshUtilities