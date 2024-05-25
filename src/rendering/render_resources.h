#pragma once

#include <rhi/ez_vulkan.h>

class UniformBuffer
{
public:
    UniformBuffer(uint32_t size);
    ~UniformBuffer();

    void write(uint8_t* data, uint32_t size, uint32_t offset = 0);

private:
    EzBuffer _buffer;
    uint8_t* _mapdata;
};