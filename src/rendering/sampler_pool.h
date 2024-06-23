#pragma once

#include <rhi/ez_vulkan.h>
#include <vector>

enum SamplerType
{
    SAMPLER_NEAREST_CLAMP = 0,
    SAMPLER_LINEAR_CLAMP = 1,
    SAMPLER_MAX_COUNT = 2,
};

class SamplerPool
{
public:
    SamplerPool();
    ~SamplerPool();

    void bind();
    EzSampler get_sampler(SamplerType type);

private:
    std::vector<EzSampler> _samplers;
};