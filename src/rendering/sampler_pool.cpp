#include "sampler_pool.h"

SamplerPool::SamplerPool()
{
    _samplers.resize(SAMPLER_MAX_COUNT);

    {
        EzSampler sampler;
        EzSamplerDesc sampler_desc{};
        sampler_desc.mag_filter = VK_FILTER_NEAREST;
        sampler_desc.min_filter = VK_FILTER_NEAREST;
        sampler_desc.address_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.address_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.address_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        ez_create_sampler(sampler_desc, sampler);
        _samplers[SAMPLER_NEAREST_CLAMP] = sampler;
    }

    {
        EzSampler sampler;
        EzSamplerDesc sampler_desc{};
        sampler_desc.address_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.address_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.address_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        ez_create_sampler(sampler_desc, sampler);
        _samplers[SAMPLER_LINEAR_CLAMP] = sampler;
    }
}

SamplerPool::~SamplerPool()
{
    for (int i = 0; i < _samplers.size(); ++i)
    {
        ez_destroy_sampler(_samplers[i]);
    }
}

void SamplerPool::bind()
{
    // Ref sampler_inc.glsl
    ez_bind_sampler(20 + SAMPLER_NEAREST_CLAMP, _samplers[SAMPLER_NEAREST_CLAMP]);
    ez_bind_sampler(20 + SAMPLER_LINEAR_CLAMP, _samplers[SAMPLER_LINEAR_CLAMP]);
}

EzSampler SamplerPool::get_sampler(SamplerType type)
{
    return _samplers[(int)type];
}