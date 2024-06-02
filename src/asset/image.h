#pragma once

#include <vector>
#include <rhi/ez_vulkan.h>

struct Image
{
    std::vector<uint8_t> data;
    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t depth = 1;
    VkFormat format;
};