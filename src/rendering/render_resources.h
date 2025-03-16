#pragma once

#include <core/module.h>
#include <rhi/ez_vulkan.h>
#include <unordered_map>

// Todo
// RenderBufferResource
// RenderTextureResource

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