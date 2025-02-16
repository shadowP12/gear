#include "render_resources.h"
#include <core/hash.h>

TextureRef::TextureRef(const EzTextureDesc& desc)
{
    _desc = desc;
    ez_create_texture(desc, _texture);
}

TextureRef::~TextureRef()
{
    ez_destroy_texture(_texture);
}

std::size_t compute_texture_ref_hash(const EzTextureDesc& desc)
{
    std::size_t hash = 0;
    hash_combine(hash, desc.width);
    hash_combine(hash, desc.height);
    hash_combine(hash, desc.depth);
    hash_combine(hash, desc.levels);
    hash_combine(hash, desc.layers);
    hash_combine(hash, desc.format);
    hash_combine(hash, desc.image_type);
    hash_combine(hash, desc.usage);
    hash_combine(hash, desc.samples);
    hash_combine(hash, desc.memory_flags);
    return hash;
}