#pragma once

#include <core/module.h>
#include <rhi/ez_vulkan.h>
#include <unordered_map>

class UniformBuffer
{
public:
    UniformBuffer(uint32_t size, bool persistent = false);
    ~UniformBuffer();

    EzBuffer get_buffer() { return _buffer; }

    void write(uint8_t* data, uint32_t size, uint32_t offset = 0);

private:
    bool _persistent;
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