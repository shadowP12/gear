#pragma once

#include "render_constants.h"
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
    EzBuffer sphere_vertex_buffer = nullptr;
    EzBuffer sphere_index_buffer = nullptr;
    float sphere_overfit = 0.0;

    EzBuffer cone_vertex_buffer = nullptr;
    EzBuffer cone_index_buffer = nullptr;
    float cone_overfit = 0.0;

    EzBuffer box_vertex_buffer = nullptr;
    EzBuffer box_index_buffer = nullptr;

    EzBuffer quad_buffer = nullptr;

private:
    std::vector<EzSampler> _samplers;
};

extern RenderSharedData* g_rsd;
