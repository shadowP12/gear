#include "sdf_generator.h"
#include "asset/asset_manager.h"
#include "asset/sdf_asset.h"

#include <core/log.h>
#include <math/detection.h>

#define SDF_RESOLUTION 32

template<typename T>
SDFAsset* generate_sdf_imp(const std::string& path, uint32_t vertex_count, float* vertices, uint32_t index_count, T* indices)
{
    BoundingBox bounds;
    for (int i = 0; i < vertex_count; ++i)
    {
        bounds.merge(glm::vec3(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]));
    }
    bounds.grow(0.02f);

    uint32_t resolution = SDF_RESOLUTION;
    float max_distance = bounds.get_max_extent();
    glm::vec3 bounds_size = bounds.get_size();
    glm::vec3 uvw_to_local_mul = bounds_size;
    glm::vec3 uvw_to_local_add = bounds.bb_min;
    glm::vec3 xyz_to_local_mul = uvw_to_local_mul / glm::vec3((float)(resolution - 1));
    glm::vec3 xyz_to_local_add = uvw_to_local_add;
    glm::vec3 local_to_uvw_mul = 1.0f / uvw_to_local_mul;
    glm::vec3 local_to_uvw_add = -uvw_to_local_add / uvw_to_local_mul;

    uint32_t voxel_data_size = resolution * resolution * resolution * sizeof(float);
    std::vector<uint8_t> voxel_data(voxel_data_size);

    uint32_t sample_count = 6;
    glm::vec3 sample_directions[6] = {
        glm::vec3(0.0f, 1.0f, 0.0f), // Up
        glm::vec3(0.0f, -1.0f, 0.0f),// Down
        glm::vec3(-1.0f, 0.0f, 0.0f),// Left
        glm::vec3(1.0f, 0.0f, 0.0f), // Right
        glm::vec3(0.0f, 0.0f, 1.0f), // Forward
        glm::vec3(0.0f, 0.0f, -1.0f) // Backward
    };

    for (int x = 0; x < resolution; ++x)
    {
        for (int y = 0; y < resolution; ++y)
        {
            for (int z = 0; z < resolution; ++z)
            {
                glm::vec3 voxel_pos = glm::vec3((float)x, (float)y, (float)z) * xyz_to_local_mul + xyz_to_local_add;
                float min_distance = max_distance;
                bool hit = false;
                float hit_distance = INF;
                glm::vec3 hit_position;
                glm::vec3 hit_normal;
                int idx0;
                int idx1;
                int idx2;
                for (int i = 0; i < index_count; i += 3)
                {
                    idx0 = (int)indices[i];
                    idx1 = (int)indices[i + 1];
                    idx2 = (int)indices[i + 2];

                    glm::vec3 v0(vertices[idx0 * 3], vertices[idx0 * 3 + 1], vertices[idx0 * 3 + 2]);
                    glm::vec3 v1(vertices[idx1 * 3], vertices[idx1 * 3 + 1], vertices[idx1 * 3 + 2]);
                    glm::vec3 v2(vertices[idx2 * 3], vertices[idx2 * 3 + 1], vertices[idx2 * 3 + 2]);
                    glm::vec3 p = closest_point_on_triangle(voxel_pos, v0, v1, v2);
                    float distance = glm::distance(p, voxel_pos);
                    if (hit_distance >= distance)
                    {
                        hit = true;
                        hit_distance = distance;
                        hit_position = p;
                    }
                }
                if (hit)
                {
                    min_distance = hit_distance;
                }

                uint32_t hit_back_count = 0, hit_count = 0;
                for (int sample = 0; sample < sample_count; sample++)
                {
                    hit = false;
                    hit_distance = INF;
                    glm::vec3 ro = voxel_pos;
                    glm::vec3 rd = sample_directions[sample];

                    for (int i = 0; i < index_count; i += 3)
                    {
                        idx0 = (int)indices[i];
                        idx1 = (int)indices[i + 1];
                        idx2 = (int)indices[i + 2];

                        glm::vec3 v0(vertices[idx0 * 3], vertices[idx0 * 3 + 1], vertices[idx0 * 3 + 2]);
                        glm::vec3 v1(vertices[idx1 * 3], vertices[idx1 * 3 + 1], vertices[idx1 * 3 + 2]);
                        glm::vec3 v2(vertices[idx2 * 3], vertices[idx2 * 3 + 1], vertices[idx2 * 3 + 2]);

                        if (ray_intersect_triangle(ro, rd, v0, v1, v2, hit_distance))
                        {
                            hit = true;
                            hit_normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
                        }
                    }

                    if (hit)
                    {
                        hit_count++;
                        if (glm::dot(rd, hit_normal) > 0)
                            hit_back_count++;
                    }
                }

                if ((float)hit_back_count > (float)sample_count * 0.2f && hit_count != 0)
                {
                    min_distance *= -1;
                }
                uint32_t z_address = resolution * resolution * z;
                uint32_t y_address = resolution * y + z_address;
                uint32_t x_address = x + y_address;
                *(voxel_data.data() + x_address) = min_distance;
            }
        }
    }

    // Gen json
    rapidjson::Document doc;
    rapidjson::StringBuffer str_buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buffer);
    SerializationContext s_ctx(&writer);
    s_ctx.object([&]() {
        s_ctx.field("data_size", voxel_data.size());
        s_ctx.field("resolution", resolution);
        s_ctx.field("local_to_uvw_mul", local_to_uvw_mul);
        s_ctx.field("local_to_uvw_add", local_to_uvw_add);
        s_ctx.field("bounds", bounds);
    });
    doc.Parse(str_buffer.GetString());
    DeserializationContext d_ctx(&doc);

    // Bin stream
    BinaryStream bin_stream(voxel_data);

    SDFAsset* sdf = AssetManager::get()->create<SDFAsset>(path);
    sdf->deserialize(d_ctx, bin_stream);
    AssetManager::get()->save(sdf);

    return sdf;
}

#define IMPLEMENT_SDF_GENERATE(IndexType)                                                       \
    SDFAsset* generate_sdf(const std::string& path,                                             \
                           uint32_t vertex_count,                                               \
                           float* vertices,                                                     \
                           uint32_t index_count,                                                \
                           IndexType* indices)                                                  \
    {                                                                                           \
        return generate_sdf_imp<IndexType>(path, vertex_count, vertices, index_count, indices); \
    }

IMPLEMENT_SDF_GENERATE(uint16_t)
IMPLEMENT_SDF_GENERATE(uint32_t)