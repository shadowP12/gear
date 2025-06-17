#include "render_shared_data.h"
#include "utils/render_utils.h"
#include <core/memory.h>
#include <math/bounding_box.h>
#include <math/plane.h>

RenderSharedData::RenderSharedData()
{
    init_samplers();
    init_geometries();
}

RenderSharedData::~RenderSharedData()
{
    clear_samplers();
    clear_geometries();
}

void RenderSharedData::init_samplers()
{
    _samplers.resize((int)SamplerType::Count);

    {
        EzSampler sampler;
        EzSamplerDesc sampler_desc{};
        sampler_desc.mag_filter = VK_FILTER_NEAREST;
        sampler_desc.min_filter = VK_FILTER_NEAREST;
        sampler_desc.address_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.address_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.address_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        ez_create_sampler(sampler_desc, sampler);
        _samplers[(int)SamplerType::NearestClamp] = sampler;
    }

    {
        EzSampler sampler;
        EzSamplerDesc sampler_desc{};
        sampler_desc.address_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.address_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.address_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        ez_create_sampler(sampler_desc, sampler);
        _samplers[(int)SamplerType::LinearClamp] = sampler;
    }
    {
        EzSampler sampler;
        EzSamplerDesc sampler_desc{};
        sampler_desc.mag_filter = VK_FILTER_LINEAR;
        sampler_desc.min_filter = VK_FILTER_LINEAR;
        sampler_desc.address_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.address_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.address_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_desc.compare_enable = true;
        sampler_desc.compare_op = VK_COMPARE_OP_LESS_OR_EQUAL;
        ez_create_sampler(sampler_desc, sampler);
        _samplers[(int)SamplerType::Shadow] = sampler;
    }
}

void RenderSharedData::clear_samplers()
{
    for (int i = 0; i < _samplers.size(); ++i)
    {
        ez_destroy_sampler(_samplers[i]);
    }
}

void RenderSharedData::init_geometries()
{
    EzBufferDesc buffer_desc = {};
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;

    // Sphere mesh
    {
        static const uint32_t icosphere_vertex_count = 42;
        static const float icosphere_vertices[icosphere_vertex_count * 3] = {
            0, 0, -1, 0.7236073, -0.5257253, -0.4472195, -0.276388, -0.8506492, -0.4472199, -0.8944262, 0, -0.4472156, -0.276388, 0.8506492, -0.4472199, 0.7236073, 0.5257253, -0.4472195, 0.276388, -0.8506492, 0.4472199, -0.7236073, -0.5257253, 0.4472195, -0.7236073, 0.5257253, 0.4472195, 0.276388, 0.8506492, 0.4472199, 0.8944262, 0, 0.4472156, 0, 0, 1, -0.1624555, -0.4999952, -0.8506544, 0.4253227, -0.3090114, -0.8506542, 0.2628688, -0.8090116, -0.5257377, 0.8506479, 0, -0.5257359, 0.4253227, 0.3090114, -0.8506542, -0.5257298, 0, -0.8506517, -0.6881894, -0.4999969, -0.5257362, -0.1624555, 0.4999952, -0.8506544, -0.6881894, 0.4999969, -0.5257362, 0.2628688, 0.8090116, -0.5257377, 0.9510579, -0.3090126, 0, 0.9510579, 0.3090126, 0, 0, -1, 0, 0.5877856, -0.8090167, 0, -0.9510579, -0.3090126, 0, -0.5877856, -0.8090167, 0, -0.5877856, 0.8090167, 0, -0.9510579, 0.3090126, 0, 0.5877856, 0.8090167, 0, 0, 1, 0, 0.6881894, -0.4999969, 0.5257362, -0.2628688, -0.8090116, 0.5257377, -0.8506479, 0, 0.5257359, -0.2628688, 0.8090116, 0.5257377, 0.6881894, 0.4999969, 0.5257362, 0.1624555, -0.4999952, 0.8506544, 0.5257298, 0, 0.8506517, -0.4253227, -0.3090114, 0.8506542, -0.4253227, 0.3090114, 0.8506542, 0.1624555, 0.4999952, 0.8506544};
        static const uint32_t icosphere_triangle_count = 80;
        static const uint16_t icosphere_triangle_indices[icosphere_triangle_count * 3] = {
            0, 13, 12, 1, 13, 15, 0, 12, 17, 0, 17, 19, 0, 19, 16, 1, 15, 22, 2, 14, 24, 3, 18, 26, 4, 20, 28, 5, 21, 30, 1, 22, 25, 2, 24, 27, 3, 26, 29, 4, 28, 31, 5, 30, 23, 6, 32, 37, 7, 33, 39, 8, 34, 40, 9, 35, 41, 10, 36, 38, 38, 41, 11, 38, 36, 41, 36, 9, 41, 41, 40, 11, 41, 35, 40, 35, 8, 40, 40, 39, 11, 40, 34, 39, 34, 7, 39, 39, 37, 11, 39, 33, 37, 33, 6, 37, 37, 38, 11, 37, 32, 38, 32, 10, 38, 23, 36, 10, 23, 30, 36, 30, 9, 36, 31, 35, 9, 31, 28, 35, 28, 8, 35, 29, 34, 8, 29, 26, 34, 26, 7, 34, 27, 33, 7, 27, 24, 33, 24, 6, 33, 25, 32, 6, 25, 22, 32, 22, 10, 32, 30, 31, 9, 30, 21, 31, 21, 4, 31, 28, 29, 8, 28, 20, 29, 20, 3, 29, 26, 27, 7, 26, 18, 27, 18, 2, 27, 24, 25, 6, 24, 14, 25, 14, 1, 25, 22, 23, 10, 22, 15, 23, 15, 5, 23, 16, 21, 5, 16, 19, 21, 19, 4, 21, 19, 20, 4, 19, 17, 20, 17, 3, 20, 17, 18, 3, 17, 12, 18, 12, 2, 18, 15, 16, 5, 15, 13, 16, 13, 0, 16, 12, 14, 2, 12, 13, 14, 13, 1, 14};

        std::vector<uint8_t> vertex_data;
        vertex_data.resize(sizeof(float) * icosphere_vertex_count * 3);
        memcpy(vertex_data.data(), icosphere_vertices, vertex_data.size());

        buffer_desc.size = vertex_data.size();
        buffer_desc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        sphere_vertex_buffer = create_render_buffer(buffer_desc, EZ_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, vertex_data.data());

        std::vector<uint8_t> index_data;
        index_data.resize(sizeof(uint16_t) * icosphere_triangle_count * 3);
        memcpy(index_data.data(), icosphere_triangle_indices, index_data.size());

        buffer_desc.size = index_data.size();
        buffer_desc.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        sphere_index_buffer = create_render_buffer(buffer_desc, EZ_RESOURCE_STATE_INDEX_BUFFER, index_data.data());

        float min_d = 1e20;
        for (uint32_t i = 0; i < icosphere_triangle_count; i++)
        {
            glm::vec3 vertices[3];
            for (uint32_t j = 0; j < 3; j++)
            {
                uint32_t index = icosphere_triangle_indices[i * 3 + j];
                for (uint32_t k = 0; k < 3; k++)
                {
                    vertices[j][k] = icosphere_vertices[index * 3 + k];
                }
            }
            Plane p(vertices[0], vertices[1], vertices[2]);
            min_d = glm::min(glm::abs(p.d), min_d);
        }
        sphere_overfit = 1.0f / min_d;
    }

    // Cone mesh
    {
        static const uint32_t cone_vertex_count = 99;
        static const float cone_vertices[cone_vertex_count * 3] = {
            0, 1, -1, 0.1950903, 0.9807853, -1, 0.3826835, 0.9238795, -1, 0.5555703, 0.8314696, -1, 0.7071068, 0.7071068, -1, 0.8314697, 0.5555702, -1, 0.9238795, 0.3826834, -1, 0.9807853, 0.1950903, -1, 1, 0, -1, 0.9807853, -0.1950902, -1, 0.9238796, -0.3826833, -1, 0.8314697, -0.5555702, -1, 0.7071068, -0.7071068, -1, 0.5555702, -0.8314697, -1, 0.3826833, -0.9238796, -1, 0.1950901, -0.9807853, -1, -3.25841e-7, -1, -1, -0.1950907, -0.9807852, -1, -0.3826839, -0.9238793, -1, -0.5555707, -0.8314693, -1, -0.7071073, -0.7071063, -1, -0.83147, -0.5555697, -1, -0.9238799, -0.3826827, -1, 0, 0, 0, -0.9807854, -0.1950894, -1, -1, 9.65599e-7, -1, -0.9807851, 0.1950913, -1, -0.9238791, 0.3826845, -1, -0.8314689, 0.5555713, -1, -0.7071059, 0.7071077, -1, -0.5555691, 0.8314704, -1, -0.3826821, 0.9238801, -1, -0.1950888, 0.9807856, -1};
        static const uint32_t cone_triangle_count = 62;
        static const uint16_t cone_triangle_indices[cone_triangle_count * 3] = {
            0, 23, 1, 1, 23, 2, 2, 23, 3, 3, 23, 4, 4, 23, 5, 5, 23, 6, 6, 23, 7, 7, 23, 8, 8, 23, 9, 9, 23, 10, 10, 23, 11, 11, 23, 12, 12, 23, 13, 13, 23, 14, 14, 23, 15, 15, 23, 16, 16, 23, 17, 17, 23, 18, 18, 23, 19, 19, 23, 20, 20, 23, 21, 21, 23, 22, 22, 23, 24, 24, 23, 25, 25, 23, 26, 26, 23, 27, 27, 23, 28, 28, 23, 29, 29, 23, 30, 30, 23, 31, 31, 23, 32, 32, 23, 0, 7, 15, 24, 32, 0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 3, 6, 7, 3, 7, 8, 9, 9, 10, 7, 10, 11, 7, 11, 12, 15, 12, 13, 15, 13, 14, 15, 15, 16, 17, 17, 18, 19, 19, 20, 24, 20, 21, 24, 21, 22, 24, 24, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 32, 1, 3, 15, 17, 24, 17, 19, 24, 24, 26, 32, 26, 28, 32, 28, 30, 32, 32, 3, 7, 7, 11, 15, 32, 7, 24};

        std::vector<uint8_t> vertex_data;
        vertex_data.resize(sizeof(float) * cone_vertex_count * 3);
        memcpy(vertex_data.data(), cone_vertices, vertex_data.size());

        buffer_desc.size = vertex_data.size();
        buffer_desc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        cone_vertex_buffer = create_render_buffer(buffer_desc, EZ_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, vertex_data.data());

        std::vector<uint8_t> index_data;
        index_data.resize(sizeof(uint16_t) * cone_triangle_count * 3);
        memcpy(index_data.data(), cone_triangle_indices, index_data.size());

        buffer_desc.size = index_data.size();
        buffer_desc.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        cone_index_buffer = create_render_buffer(buffer_desc, EZ_RESOURCE_STATE_INDEX_BUFFER, index_data.data());

        float min_d = 1e20;
        for (uint32_t i = 0; i < cone_triangle_count; i++)
        {
            glm::vec3 vertices[3];
            int32_t zero_index = -1;
            for (uint32_t j = 0; j < 3; j++)
            {
                uint32_t index = cone_triangle_indices[i * 3 + j];
                for (uint32_t k = 0; k < 3; k++)
                {
                    vertices[j][k] = cone_vertices[index * 3 + k];
                }
                if (vertices[j] == glm::vec3(0.0f))
                {
                    zero_index = j;
                }
            }

            if (zero_index != -1)
            {
                glm::vec3 a = vertices[(zero_index + 1) % 3];
                glm::vec3 b = vertices[(zero_index + 2) % 3];
                glm::vec3 c = a + glm::vec3(0, 0, 1);
                Plane p(a, b, c);
                min_d = glm::min(glm::abs(p.d), min_d);
            }
        }
        cone_overfit = 1.0f / min_d;
    }

    // Box mesh
    {
        static const uint32_t box_vertex_count = 8;
        static const float box_vertices[box_vertex_count * 3] = {
            -1, -1, -1, -1, -1, 1, -1, 1, -1, -1, 1, 1, 1, -1, -1, 1, -1, 1, 1, 1, -1, 1, 1, 1};
        static const uint32_t box_triangle_count = 12;
        static const uint16_t box_triangle_indices[box_triangle_count * 3] = {
            1, 2, 0, 3, 6, 2, 7, 4, 6, 5, 0, 4, 6, 0, 2, 3, 5, 7, 1, 3, 2, 3, 7, 6, 7, 5, 4, 5, 1, 0, 6, 4, 0, 3, 1, 5};

        std::vector<uint8_t> vertex_data;
        vertex_data.resize(sizeof(float) * box_vertex_count * 3);
        memcpy(vertex_data.data(), box_vertices, vertex_data.size());

        buffer_desc.size = vertex_data.size();
        buffer_desc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        box_vertex_buffer = create_render_buffer(buffer_desc, EZ_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, vertex_data.data());

        std::vector<uint8_t> index_data;
        index_data.resize(sizeof(uint16_t) * box_triangle_count * 3);
        memcpy(index_data.data(), box_triangle_indices, index_data.size());

        buffer_desc.size = index_data.size();
        buffer_desc.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        box_index_buffer = create_render_buffer(buffer_desc, EZ_RESOURCE_STATE_INDEX_BUFFER, index_data.data());
    }

    {
        static float quad_vertices[] = {
            // positions        // texcoords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

        buffer_desc.size = sizeof(quad_vertices);
        buffer_desc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        quad_buffer = create_render_buffer(buffer_desc, EZ_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, quad_vertices);
    }
}

void RenderSharedData::clear_geometries()
{
    ez_destroy_buffer(sphere_vertex_buffer);
    ez_destroy_buffer(sphere_index_buffer);

    ez_destroy_buffer(cone_vertex_buffer);
    ez_destroy_buffer(cone_index_buffer);

    ez_destroy_buffer(box_vertex_buffer);
    ez_destroy_buffer(box_index_buffer);

    ez_destroy_buffer(quad_buffer);
}

EzSampler RenderSharedData::get_sampler(SamplerType type)
{
    return _samplers[(int)type];
}

RenderSharedData* g_rsd = nullptr;