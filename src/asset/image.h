#pragma once

#include <vector>
#include <rhi/ez_vulkan.h>

struct Image
{
    uint8_t* data = nullptr;
    uint32_t data_size = 0;
    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t depth = 1;
    VkFormat format;
};