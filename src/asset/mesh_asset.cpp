#include "mesh_asset.h"
#include "material_asset.h"
#include "asset_manager.h"
#include "rendering/utils/render_utils.h"
#include <core/memory.h>

EzBuffer create_mesh_buffer(void* data, uint32_t data_size, VkBufferUsageFlags usage)
{
    EzBufferDesc buffer_desc = {};
    buffer_desc.size = data_size;
    buffer_desc.usage = usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
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
}

MeshAsset::MeshAsset(const std::string& asset_path)
    : Asset(asset_path)
{
}

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

        SAFE_DELETE(surface);
    }
    _surfaces.clear();
}

void MeshAsset::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("data_size");
    writer.Int(_data.size());

    writer.Key("surfaces");
    writer.StartArray();
    for (int i = 0; i < _surfaces.size(); ++i)
    {
        Surface* surface = _surfaces[i];
        writer.StartObject();
        writer.Key("total_size");
        writer.Int(surface->total_size);
        writer.Key("total_offset");
        writer.Int(surface->total_offset);
        writer.Key("vertex_count");
        writer.Int(surface->vertex_count);
        writer.Key("vertex_offset");
        writer.Int(surface->vertex_offset);
        writer.Key("vertex_size");
        writer.Int(surface->vertex_size);
        writer.Key("position_size");
        writer.Int(surface->position_size);
        writer.Key("position_offset");
        writer.Int(surface->position_offset);
        writer.Key("normal_size");
        writer.Int(surface->normal_size);
        writer.Key("normal_offset");
        writer.Int(surface->normal_offset);
        writer.Key("tangent_size");
        writer.Int(surface->tangent_size);
        writer.Key("tangent_offset");
        writer.Int(surface->tangent_offset);
        writer.Key("uv0_size");
        writer.Int(surface->uv0_size);
        writer.Key("uv0_offset");
        writer.Int(surface->uv0_offset);
        writer.Key("uv1_size");
        writer.Int(surface->uv1_size);
        writer.Key("uv1_offset");
        writer.Int(surface->uv1_offset);
        writer.Key("index_count");
        writer.Int(surface->index_count);
        writer.Key("index_size");
        writer.Int(surface->index_size);
        writer.Key("index_offset");
        writer.Int(surface->index_offset);
        writer.Key("primitive_topology");
        writer.Int(surface->primitive_topology);
        writer.Key("using_16u");
        writer.Bool(surface->using_16u);

        writer.Key("maxp");
        Serialization::w_vec3(writer, surface->bounding_box.bb_max);

        writer.Key("minp");
        Serialization::w_vec3(writer, surface->bounding_box.bb_min);

        writer.Key("material");
        writer.String(surface->material->get_asset_path().c_str());

        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    bin.write(_data.data(), _data.size());
}

void MeshAsset::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
     uint32_t data_size = value["data_size"].GetInt();
     uint8_t* data = bin.read(data_size);
     _data.insert(_data.end(), data, data + data_size);

     for(int i = 0; i < value["surfaces"].Size(); i++)
     {
        rapidjson::Value& surfaces_value = value["surfaces"][i];
        Surface* surface = new Surface();
        surface->total_size = surfaces_value["total_size"].GetInt();
        surface->total_offset = surfaces_value["total_offset"].GetInt();
        surface->vertex_count = surfaces_value["vertex_count"].GetInt();
        surface->vertex_size = surfaces_value["vertex_size"].GetInt();
        surface->vertex_offset = surfaces_value["vertex_offset"].GetInt();
        surface->position_size = surfaces_value["position_size"].GetInt();
        surface->position_offset = surfaces_value["position_offset"].GetInt();
        surface->normal_size = surfaces_value["normal_size"].GetInt();
        surface->normal_offset = surfaces_value["normal_offset"].GetInt();
        surface->tangent_size = surfaces_value["tangent_size"].GetInt();
        surface->tangent_offset = surfaces_value["tangent_offset"].GetInt();
        surface->uv0_size = surfaces_value["uv0_size"].GetInt();
        surface->uv0_offset = surfaces_value["uv0_offset"].GetInt();
        surface->uv1_size = surfaces_value["uv1_size"].GetInt();
        surface->uv1_offset = surfaces_value["uv1_offset"].GetInt();
        surface->index_count = surfaces_value["index_count"].GetInt();
        surface->index_size = surfaces_value["index_size"].GetInt();
        surface->index_offset = surfaces_value["index_offset"].GetInt();
        surface->using_16u = surfaces_value["using_16u"].GetBool();
        surface->primitive_topology = (VkPrimitiveTopology)surfaces_value["primitive_topology"].GetInt();
        generate_surface_buffer(surface, data);

        glm::vec3 maxp = Serialization::r_vec3(surfaces_value["maxp"]);
        glm::vec3 minp = Serialization::r_vec3(surfaces_value["minp"]);
        surface->bounding_box.merge(maxp);
        surface->bounding_box.merge(minp);

        surface->material = AssetManager::get()->load<MaterialAsset>(surfaces_value["material"].GetString());

        _surfaces.push_back(surface);
     }
}