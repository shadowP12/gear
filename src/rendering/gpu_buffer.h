#pragma once

#include <core/enum_flag.h>
#include <rhi/ez_vulkan.h>

enum class BufferUsageFlags : uint32_t
{
    Static =    1 << 0,
    Dynamic =   1 << 1,
    Vertex=     1 << 2,
    Index=      1 << 3,
    Uniform =   1 << 4,
    Storage =   1 << 5
};
SP_MAKE_ENUM_FLAG(uint32_t, BufferUsageFlags)

class GpuBuffer
{
public:
    GpuBuffer(BufferUsageFlags flags, uint32_t size, uint8_t* data = nullptr);

    virtual ~GpuBuffer();

    EzBuffer get_handle() { return _buffer; }

    void clear(uint32_t size, uint32_t offset = 0);

    void write(uint8_t* data, uint32_t size, uint32_t offset = 0);

protected:
    bool _cpu_access;
    EzBuffer _buffer;
    uint8_t* _mapdata;
    EzResourceState _dst_state;
};