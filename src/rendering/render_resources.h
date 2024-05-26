#pragma once

#include <rhi/ez_vulkan.h>

class UniformBuffer
{
public:
    UniformBuffer(uint32_t size);
    ~UniformBuffer();

    EzBuffer get_buffer() { return _buffer; }

    void write(uint8_t* data, uint32_t size, uint32_t offset = 0);

private:
    EzBuffer _buffer;
    uint8_t* _mapdata;
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