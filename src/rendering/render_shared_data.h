#pragma once

#include "render_constants.h"
#include "render_resources.h"
#include <rhi/ez_vulkan.h>
#include <vector>

class RenderSharedData
{
public:
    RenderSharedData();
    ~RenderSharedData();

    void bind_samplers();
    EzSampler get_sampler(SamplerType type);

private:
    void init_samplers();
    void clear_samplers();

    void init_geometries();
    void clear_geometries();

public:
    VertexBuffer* sphere_vertex_buffer = nullptr;
    IndexBuffer* sphere_index_buffer = nullptr;
    float sphere_overfit = 0.0;

    VertexBuffer* cone_vertex_buffer = nullptr;
    IndexBuffer* cone_index_buffer = nullptr;
    float cone_overfit = 0.0;

    VertexBuffer* box_vertex_buffer = nullptr;
    IndexBuffer* box_index_buffer = nullptr;

private:
    std::vector<EzSampler> _samplers;
};