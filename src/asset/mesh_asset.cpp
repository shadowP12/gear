#include "mesh_asset.h"
#include "asset_manager.h"
#include "sdf_asset.h"
#include "material_asset.h"
#include "rendering/utils/render_utils.h"

#include <core/memory.h>

EzBuffer create_mesh_buffer(void* data, uint32_t data_size, VkBufferUsageFlags usage)
{
    EzBufferDesc buffer_desc = {};
    buffer_desc.size = data_size;
    buffer_desc.usage = usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (ez_support_feature(EZ_FEATURE_RAYTRACING))
    {
        buffer_desc.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
        buffer_desc.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    }

    buffer_desc.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;

    EzResourceState flag = EZ_RESOURCE_STATE_UNDEFINED;
    if ((usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) != 0)
        flag |= EZ_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if ((usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) != 0)
        flag |= EZ_RESOURCE_STATE_INDEX_BUFFER;
    if ((usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) != 0)
        flag |= EZ_RESOURCE_STATE_INDIRECT_ARGUMENT;

    return create_render_buffer(buffer_desc, flag, data);
}

void generate_surface_buffer(MeshAsset::Surface* surface, uint8_t* data)
{
    surface->vertex_buffer_count = 4;
    surface->vertex_buffers[0] = create_mesh_buffer(data + surface->position_offset, surface->position_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    surface->vertex_buffers[1] = create_mesh_buffer(data + surface->normal_offset, surface->normal_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    surface->vertex_buffers[2] = create_mesh_buffer(data + surface->tangent_offset, surface->tangent_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    surface->vertex_buffers[3] = create_mesh_buffer(data + surface->uv0_offset, surface->uv0_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    surface->index_count = surface->index_count;
    surface->index_type = surface->using_16u ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    surface->index_buffer = create_mesh_buffer(data + surface->index_offset, surface->index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    if (ez_support_feature(EZ_FEATURE_RAYTRACING))
    {
        EzAccelerationStructureBuildInfo blas_info = {};
        blas_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        blas_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
        blas_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

        EzAccelerationStructureTriangles triangles;
        triangles.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        triangles.vertex_format = VK_FORMAT_R32G32B32_SFLOAT;
        triangles.vertex_count = surface->vertex_count;
        triangles.vertex_offset = 0;
        triangles.vertex_stride = sizeof(glm::vec3);
        triangles.vertex_buffer = surface->vertex_buffers[0];
        triangles.index_type = surface->index_type;
        triangles.index_count = surface->index_count;
        triangles.index_offset = 0;
        triangles.index_buffer = surface->index_buffer;
        blas_info.geometry_set.triangles.push_back(triangles);

        ez_create_acceleration_structure(blas_info, surface->blas);
        ez_build_acceleration_structure(blas_info, surface->blas);
    }
}

MeshAsset::MeshAsset(const std::string& asset_path)
    : Asset(asset_path) {}

MeshAsset::~MeshAsset()
{
    for (auto surface : _surfaces)
    {
        for (int i = 0; i < surface->vertex_buffer_count; ++i)
        {
            ez_destroy_buffer(surface->vertex_buffers[i]);
        }
        if (surface->index_buffer)
        {
            ez_destroy_buffer(surface->index_buffer);
        }
        if (surface->blas)
        {
            ez_destroy_acceleration_structure(surface->blas);
        }

        SAFE_DELETE(surface);
    }
    _surfaces.clear();
}

void MeshAsset::serialize(SerializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.object([&]() {
        ctx.key("data_size");
        ctx.field(_data.size());
        ctx.array("surfaces", [&]() {
            for (size_t i = 0; i < _surfaces.size(); ++i)
            {
                Surface* surface = _surfaces[i];
                ctx.object([&]() {
                    ctx.field("total_size", surface->total_size);
                    ctx.field("total_offset", surface->total_offset);
                    ctx.field("vertex_count", surface->vertex_count);
                    ctx.field("vertex_size", surface->vertex_size);
                    ctx.field("vertex_offset", surface->vertex_offset);
                    ctx.field("position_size", surface->position_size);
                    ctx.field("position_offset", surface->position_offset);
                    ctx.field("normal_size", surface->normal_size);
                    ctx.field("normal_offset", surface->normal_offset);
                    ctx.field("tangent_size", surface->tangent_size);
                    ctx.field("tangent_offset", surface->tangent_offset);
                    ctx.field("uv0_size", surface->uv0_size);
                    ctx.field("uv0_offset", surface->uv0_offset);
                    ctx.field("uv1_size", surface->uv1_size);
                    ctx.field("uv1_offset", surface->uv1_offset);
                    ctx.field("index_count", surface->index_count);
                    ctx.field("index_size", surface->index_size);
                    ctx.field("index_offset", surface->index_offset);
                    ctx.field("using_16u", surface->using_16u);
                    ctx.field("primitive_topology", surface->primitive_topology);
                    ctx.field("bounds", surface->bounding_box);
                    ctx.field("material", surface->material->get_asset_path());
                    if (surface->sdf)
                    {
                        ctx.field("sdf", surface->sdf->get_asset_path());
                    }
                });
            }
        });
    });

    bin_stream.write(_data.data(), _data.size());
}

void MeshAsset::deserialize(DeserializationContext& ctx, BinaryStream& bin_stream)
{
    uint32_t data_size;
    ctx.field("data_size", data_size);
    uint8_t* data = bin_stream.read(data_size);
    _data.insert(_data.end(), data, data + data_size);

    ctx.array("surfaces", [&]() {
        Surface* surface = new Surface();
        ctx.field("total_size", surface->total_size);
        ctx.field("total_offset", surface->total_offset);
        ctx.field("vertex_count", surface->vertex_count);
        ctx.field("vertex_size", surface->vertex_size);
        ctx.field("vertex_offset", surface->vertex_offset);
        ctx.field("position_size", surface->position_size);
        ctx.field("position_offset", surface->position_offset);
        ctx.field("normal_size", surface->normal_size);
        ctx.field("normal_offset", surface->normal_offset);
        ctx.field("tangent_size", surface->tangent_size);
        ctx.field("tangent_offset", surface->tangent_offset);
        ctx.field("uv0_size", surface->uv0_size);
        ctx.field("uv0_offset", surface->uv0_offset);
        ctx.field("uv1_size", surface->uv1_size);
        ctx.field("uv1_offset", surface->uv1_offset);
        ctx.field("index_count", surface->index_count);
        ctx.field("index_size", surface->index_size);
        ctx.field("index_offset", surface->index_offset);
        ctx.field("using_16u", surface->using_16u);
        ctx.field("primitive_topology", surface->primitive_topology);
        ctx.field("bounds", surface->bounding_box);
        generate_surface_buffer(surface, data);

        std::string material_url;
        ctx.field("material", material_url);
        surface->material = AssetManager::get()->load<MaterialAsset>(material_url);

        if (ctx.has_key("sdf"))
        {
            std::string sdf_url;
            ctx.field("sdf", sdf_url);
            surface->sdf = AssetManager::get()->load<SDFAsset>(sdf_url);
        }

        _surfaces.push_back(surface);
    });
}