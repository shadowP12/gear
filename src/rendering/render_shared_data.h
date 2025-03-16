#pragma once

#include "gpu_buffer.h"
#include "render_constants.h"
#include "render_resources.h"
#include <rhi/ez_vulkan.h>
#include <vector>

class RenderSharedData
{
public:
    RenderSharedData();
    ~RenderSharedData();

    EzSampler get_sampler(SamplerType type);

private:
    void init_samplers();
    void clear_samplers();

    void init_geometries();
    void clear_geometries();

public:
    GpuBuffer* sphere_vertex_buffer = nullptr;
    GpuBuffer* sphere_index_buffer = nullptr;
    float sphere_overfit = 0.0;

    GpuBuffer* cone_vertex_buffer = nullptr;
    GpuBuffer* cone_index_buffer = nullptr;
    float cone_overfit = 0.0;

    GpuBuffer* box_vertex_buffer = nullptr;
    GpuBuffer* box_index_buffer = nullptr;

private:
    std::vector<EzSampler> _samplers;
};

extern RenderSharedData* g_rsd;
