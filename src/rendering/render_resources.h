#pragma once

#include <core/module.h>
#include <rhi/ez_vulkan.h>
#include <unordered_map>

class RenderBuffer
{
public:
    RenderBuffer(VkBufferUsageFlags usage, EzResourceState dst_state, uint32_t size, bool persistent = false);
    virtual ~RenderBuffer();

    EzBuffer get_buffer() { return _buffer; }

    void write(uint8_t* data, uint32_t size, uint32_t offset = 0);

protected:
    bool _persistent;
    EzBuffer _buffer;
    uint8_t* _mapdata;
    VkBufferUsageFlags _usage;
    EzResourceState _dst_state;
};

class UniformBuffer : public RenderBuffer
{
public:
    UniformBuffer(uint32_t size, bool persistent = false);
    virtual ~UniformBuffer();
};

class VertexBuffer : public RenderBuffer
{
public:
    VertexBuffer(uint32_t size, bool persistent = false);
    virtual ~VertexBuffer();
};

class IndexBuffer : public RenderBuffer
{
public:
    IndexBuffer(uint32_t size, bool persistent = false);
    virtual ~IndexBuffer();
};

class TextureRef
{
public:
    TextureRef(const EzTextureDesc& desc);
    ~TextureRef();

    EzTexture get_texture() { return _texture; }
    const EzTextureDesc& get_desc() { return _desc; }

private:
    EzTexture _texture;
    EzTextureDesc _desc;
};

std::size_t compute_texture_ref_hash(const EzTextureDesc& desc);