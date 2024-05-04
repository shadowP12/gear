#include "mesh.h"

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

void Mesh::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, std::vector<uint8_t>& bin)
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
        writer.StartArray();
        writer.Double(sub_mesh->bounding_box.bb_max[0]);
        writer.Double(sub_mesh->bounding_box.bb_max[1]);
        writer.Double(sub_mesh->bounding_box.bb_max[2]);
        writer.EndArray();

        writer.Key("minp");
        writer.StartArray();
        writer.Double(sub_mesh->bounding_box.bb_min[0]);
        writer.Double(sub_mesh->bounding_box.bb_min[1]);
        writer.Double(sub_mesh->bounding_box.bb_min[2]);
        writer.EndArray();
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    bin.insert(bin.end(), _data.data(), _data.data() + _data.size());
}

void Mesh::deserialize(const rapidjson::Value& reader, const std::vector<uint8_t>& bin)
{
     uint32_t data_size = reader["data_size"].GetInt();
     _data.insert(_data.end(), bin.data(), bin.data() + data_size);

     for(int i = 0; i < reader["sub_meshes"].Size(); i++)
     {
        SubMesh* sub_mesh = new SubMesh();
        sub_mesh->total_size = reader["sub_meshes"][i]["total_size"].GetInt();
        sub_mesh->total_offset = reader["sub_meshes"][i]["total_offset"].GetInt();
        sub_mesh->vertex_count = reader["sub_meshes"][i]["vertex_count"].GetInt();
        sub_mesh->vertex_size = reader["sub_meshes"][i]["vertex_size"].GetInt();
        sub_mesh->position_size = reader["sub_meshes"][i]["position_size"].GetInt();
        sub_mesh->position_offset = reader["sub_meshes"][i]["position_offset"].GetInt();
        sub_mesh->normal_size = reader["sub_meshes"][i]["normal_size"].GetInt();
        sub_mesh->normal_offset = reader["sub_meshes"][i]["normal_offset"].GetInt();
        sub_mesh->tangent_size = reader["sub_meshes"][i]["tangent_size"].GetInt();
        sub_mesh->tangent_offset = reader["sub_meshes"][i]["tangent_offset"].GetInt();
        sub_mesh->uv_size = reader["sub_meshes"][i]["uv_size"].GetInt();
        sub_mesh->uv_offset = reader["sub_meshes"][i]["uv_offset"].GetInt();
        sub_mesh->index_count = reader["sub_meshes"][i]["index_count"].GetInt();
        sub_mesh->index_size = reader["sub_meshes"][i]["index_size"].GetInt();
        sub_mesh->index_offset = reader["sub_meshes"][i]["index_offset"].GetInt();

        if (reader["sub_meshes"][i]["using_16u"].GetBool()) {
            sub_mesh->index_type = VK_INDEX_TYPE_UINT16;
        } else {
            sub_mesh->index_type = VK_INDEX_TYPE_UINT32;
        }

        const rapidjson::Value& json_maxp = reader["sub_meshes"][i]["maxp"].GetArray();
        const rapidjson::Value& json_minp = reader["sub_meshes"][i]["minp"].GetArray();
        glm::vec3 maxp = glm::vec3(json_maxp[0].GetDouble(), json_maxp[1].GetDouble(), json_maxp[2].GetDouble());
        glm::vec3 minp = glm::vec3(json_minp[0].GetDouble(), json_minp[1].GetDouble(), json_minp[2].GetDouble());
        sub_mesh->bounding_box.merge(maxp);
        sub_mesh->bounding_box.merge(minp);

        _sub_meshes.push_back(sub_mesh);
     }
}