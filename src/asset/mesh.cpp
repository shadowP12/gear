#include "mesh.h"
#include "material.h"
#include "asset_manager.h"
#include "mesh_utilities.h"
#include "rendering/vertex_factory.h"

Mesh::Mesh(const std::string& asset_path)
    : Asset(asset_path)
{
}

Mesh::~Mesh()
{
    for (auto primitive : _primitives)
    {
        ez_destroy_buffer(primitive->vertex_buffer);
        ez_destroy_buffer(primitive->index_buffer);
        delete primitive;
    }
    _primitives.clear();
}

void Mesh::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("data_size");
    writer.Int(_data.size());

    writer.Key("primitives");
    writer.StartArray();
    for (int i = 0; i < _primitives.size(); ++i)
    {
        MeshPrimitive* primitive = _primitives[i];
        writer.StartObject();
        writer.Key("total_size");
        writer.Int(primitive->total_size);
        writer.Key("total_offset");
        writer.Int(primitive->total_offset);
        writer.Key("vertex_count");
        writer.Int(primitive->vertex_count);
        writer.Key("vertex_offset");
        writer.Int(primitive->vertex_offset);
        writer.Key("vertex_size");
        writer.Int(primitive->vertex_size);
        writer.Key("index_count");
        writer.Int(primitive->index_count);
        writer.Key("index_size");
        writer.Int(primitive->index_size);
        writer.Key("index_offset");
        writer.Int(primitive->index_offset);
        writer.Key("primitive_topology");
        writer.Int(primitive->primitive_topology);
        writer.Key("vertex_factory");
        writer.Int(primitive->vertex_factory);

        if (primitive->index_type == VK_INDEX_TYPE_UINT16)
        {
            writer.Key("using_16u");
            writer.Bool(true);
        }
        else
        {
            writer.Key("using_16u");
            writer.Bool(false);
        }

        writer.Key("maxp");
        Serialization::w_vec3(writer, primitive->bounding_box.bb_max);

        writer.Key("minp");
        Serialization::w_vec3(writer, primitive->bounding_box.bb_min);

        writer.Key("material");
        writer.String(primitive->material->get_asset_path().c_str());

        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    bin.write(_data.data(), _data.size());
}

void Mesh::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
     uint32_t data_size = value["data_size"].GetInt();
     uint8_t* data = bin.read(data_size);
     _data.insert(_data.end(), data, data + data_size);

     for(int i = 0; i < value["primitives"].Size(); i++)
     {
        rapidjson::Value& primitive_value = value["primitives"][i];
        MeshPrimitive* primitive = new MeshPrimitive();
        primitive->total_size = primitive_value["total_size"].GetInt();
        primitive->total_offset = primitive_value["total_offset"].GetInt();
        primitive->vertex_count = primitive_value["vertex_count"].GetInt();
        primitive->vertex_size = primitive_value["vertex_size"].GetInt();
        primitive->vertex_offset = primitive_value["vertex_offset"].GetInt();
        primitive->index_count = primitive_value["index_count"].GetInt();
        primitive->index_size = primitive_value["index_size"].GetInt();
        primitive->index_offset = primitive_value["index_offset"].GetInt();
        primitive->vertex_factory = primitive_value["vertex_factory"].GetInt();
        primitive->primitive_topology = (VkPrimitiveTopology)primitive_value["primitive_topology"].GetInt();

        if (primitive_value["using_16u"].GetBool())
        {
            primitive->index_type = VK_INDEX_TYPE_UINT16;
        }
        else
        {
            primitive->index_type = VK_INDEX_TYPE_UINT32;
        }

        glm::vec3 maxp = Serialization::r_vec3(primitive_value["maxp"]);
        glm::vec3 minp = Serialization::r_vec3(primitive_value["minp"]);
        primitive->bounding_box.merge(maxp);
        primitive->bounding_box.merge(minp);

        primitive->material = AssetManager::get()->load<Material>(primitive_value["material"].GetString());

        _primitives.push_back(primitive);
     }

     generate_mesh_buffers();
}

void Mesh::generate_mesh_buffers()
{
     for (auto primitive : _primitives)
     {
        primitive->vertex_buffer = MeshUtilities::create_mesh_buffer(_data.data() + primitive->vertex_offset, primitive->vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        primitive->index_buffer = MeshUtilities::create_mesh_buffer(_data.data() + primitive->index_offset, primitive->index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
     }
}