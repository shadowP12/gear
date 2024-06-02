#include "mesh.h"
#include "material.h"
#include "asset_manager.h"

Mesh::Mesh(const std::string& asset_path)
    : Asset(asset_path)
{
}

Mesh::~Mesh()
{
    for (auto sub_mesh : _sub_meshes)
    {
        delete sub_mesh;
    }
    _sub_meshes.clear();
}

void Mesh::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("data_size");
    writer.Int(_data.size());

    writer.Key("sub_meshes");
    writer.StartArray();
    for (int i = 0; i < _sub_meshes.size(); ++i)
    {
        SubMesh* sub_mesh = _sub_meshes[i];
        writer.StartObject();
        writer.Key("total_size");
        writer.Int(sub_mesh->total_size);
        writer.Key("total_offset");
        writer.Int(sub_mesh->total_offset);
        writer.Key("vertex_count");
        writer.Int(sub_mesh->vertex_count);
        writer.Key("vertex_size");
        writer.Int(sub_mesh->vertex_size);
        writer.Key("position_size");
        writer.Int(sub_mesh->position_size);
        writer.Key("position_offset");
        writer.Int(sub_mesh->position_offset);
        writer.Key("normal_size");
        writer.Int(sub_mesh->normal_size);
        writer.Key("normal_offset");
        writer.Int(sub_mesh->normal_offset);
        writer.Key("tangent_size");
        writer.Int(sub_mesh->tangent_size);
        writer.Key("tangent_offset");
        writer.Int(sub_mesh->tangent_offset);
        writer.Key("uv_size");
        writer.Int(sub_mesh->uv_size);
        writer.Key("uv_offset");
        writer.Int(sub_mesh->uv_offset);
        writer.Key("index_count");
        writer.Int(sub_mesh->index_count);
        writer.Key("index_size");
        writer.Int(sub_mesh->index_size);
        writer.Key("index_offset");
        writer.Int(sub_mesh->index_offset);

        if (sub_mesh->index_type == VK_INDEX_TYPE_UINT16)
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
        Serialization::w_vec3(writer, sub_mesh->bounding_box.bb_max);

        writer.Key("minp");
        Serialization::w_vec3(writer, sub_mesh->bounding_box.bb_min);

        writer.Key("material");
        writer.String(sub_mesh->material->get_asset_path().c_str());

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

     for(int i = 0; i < value["sub_meshes"].Size(); i++)
     {
        SubMesh* sub_mesh = new SubMesh();
        sub_mesh->total_size = value["sub_meshes"][i]["total_size"].GetInt();
        sub_mesh->total_offset = value["sub_meshes"][i]["total_offset"].GetInt();
        sub_mesh->vertex_count = value["sub_meshes"][i]["vertex_count"].GetInt();
        sub_mesh->vertex_size = value["sub_meshes"][i]["vertex_size"].GetInt();
        sub_mesh->position_size = value["sub_meshes"][i]["position_size"].GetInt();
        sub_mesh->position_offset = value["sub_meshes"][i]["position_offset"].GetInt();
        sub_mesh->normal_size = value["sub_meshes"][i]["normal_size"].GetInt();
        sub_mesh->normal_offset = value["sub_meshes"][i]["normal_offset"].GetInt();
        sub_mesh->tangent_size = value["sub_meshes"][i]["tangent_size"].GetInt();
        sub_mesh->tangent_offset = value["sub_meshes"][i]["tangent_offset"].GetInt();
        sub_mesh->uv_size = value["sub_meshes"][i]["uv_size"].GetInt();
        sub_mesh->uv_offset = value["sub_meshes"][i]["uv_offset"].GetInt();
        sub_mesh->index_count = value["sub_meshes"][i]["index_count"].GetInt();
        sub_mesh->index_size = value["sub_meshes"][i]["index_size"].GetInt();
        sub_mesh->index_offset = value["sub_meshes"][i]["index_offset"].GetInt();

        if (value["sub_meshes"][i]["using_16u"].GetBool()) {
            sub_mesh->index_type = VK_INDEX_TYPE_UINT16;
        } else {
            sub_mesh->index_type = VK_INDEX_TYPE_UINT32;
        }

        glm::vec3 maxp = Serialization::r_vec3(value["sub_meshes"][i]["maxp"]);
        glm::vec3 minp = Serialization::r_vec3(value["sub_meshes"][i]["minp"]);
        sub_mesh->bounding_box.merge(maxp);
        sub_mesh->bounding_box.merge(minp);

        sub_mesh->material = AssetManager::get()->load<Material>(value["sub_meshes"][i]["material"].GetString());

        _sub_meshes.push_back(sub_mesh);
     }

     generate_mesh_buffers();
}

void Mesh::generate_mesh_buffers()
{

}