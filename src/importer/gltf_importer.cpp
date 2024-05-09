#include "gltf_importer.h"
#include "asset/asset.h"
#include "asset/texture2d.h"
#include "asset/material.h"
#include "asset/mesh.h"
#include "asset/asset_manager.h"
#include <core/path.h>
#include <core/io/dir_access.h>
#include <math/bounding_box.h>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

struct GltfSubMesh
{
    uint32_t total_size;
    uint32_t total_offset;
    uint32_t vertex_count;
    uint32_t vertex_size;
    uint32_t position_size;
    uint32_t position_offset;
    uint32_t normal_size;
    uint32_t normal_offset;
    uint32_t tangent_size;
    uint32_t tangent_offset;
    uint32_t uv_size;
    uint32_t uv_offset;
    uint32_t index_count;
    uint32_t index_size;
    uint32_t index_offset;
    bool using_16u_index;
    BoundingBox bounding_box;
};

glm::mat4 get_local_matrix(cgltf_node* node)
{
    glm::vec3 translation = glm::vec3(0.0f);
    if (node->has_translation)
    {
        translation.x = node->translation[0];
        translation.y = node->translation[1];
        translation.z = node->translation[2];
    }

    glm::quat rotation = glm::quat(1, 0, 0, 0);
    if (node->has_rotation)
    {
        rotation.x = node->rotation[0];
        rotation.y = node->rotation[1];
        rotation.z = node->rotation[2];
        rotation.w = node->rotation[3];
    }

    glm::vec3 scale = glm::vec3(1.0f);
    if (node->has_scale)
    {
        scale.x = node->scale[0];
        scale.y = node->scale[1];
        scale.z = node->scale[2];
    }

    glm::mat4 r, t, s;
    r = glm::toMat4(rotation);
    t = glm::translate(glm::mat4(1.0), translation);
    s = glm::scale(glm::mat4(1.0), scale);
    return t * r * s;
}

glm::mat4 get_world_matrix(cgltf_node* node)
{
    cgltf_node* cur_node = node;
    glm::mat4 out = get_local_matrix(cur_node);

    while (cur_node->parent != nullptr)
    {
        cur_node = node->parent;
        out = get_local_matrix(cur_node) * out;
    }
    return out;
}

cgltf_attribute* get_gltf_attribute(cgltf_primitive* primitive, cgltf_attribute_type type)
{
    for (int i = 0; i < primitive->attributes_count; i++)
    {
        cgltf_attribute* att = &primitive->attributes[i];
        if(att->type == type)
            return att;
    }
    return nullptr;
}

void GltfImporter::import_asset(const std::string& file_path, const std::string& output_path)
{
    cgltf_options options = {static_cast<cgltf_file_type>(0)};
    cgltf_data* data = NULL;
    if (cgltf_parse_file(&options, file_path.c_str(), &data) != cgltf_result_success)
    {
        cgltf_free(data);
        return;
    }

    if (cgltf_load_buffers(&options, data, file_path.c_str()) != cgltf_result_success)
    {
        cgltf_free(data);
        return;
    }

    if (cgltf_validate(data) != cgltf_result_success)
    {
        cgltf_free(data);
        return;
    }

    // Load gltf textures
    std::map<cgltf_image*, std::string> image_helper;
    for (int i = 0; i < data->images_count; ++i)
    {
        cgltf_image* cimage = &data->images[i];
        std::string input_image_path = Path::parent_path(file_path) + "/" + cimage->uri;
        std::string output_image_path = output_path + "/textures/" + Path::filename(cimage->uri);
        std::string asset_path = output_image_path;

        if (!AssetManager::get()->exist_asset(asset_path))
        {
            // Copy raw file
            if (!FileAccess::exist(output_image_path)) {
                if (!DirAccess::dir_exists(Path::parent_path(output_image_path))) {
                    DirAccess::make_dir_recursive(Path::parent_path(output_image_path));
                }
                DirAccess::copy(input_image_path, output_image_path);
            }

            // Gen json
            rapidjson::Document doc;
            rapidjson::StringBuffer str_buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buffer);
            writer.StartObject();
            writer.Key("uri");
            writer.String(Path::filename(output_image_path).c_str());
            writer.EndObject();
            doc.Parse(str_buffer.GetString());

            Texture2D* tex_2d = AssetManager::get()->create<Texture2D>(asset_path);
            tex_2d->deserialize(doc.GetObject(), std::vector<uint8_t>());
            AssetManager::get()->save(tex_2d);
        }

        image_helper[cimage] = asset_path;
    }

    // Load gltf material
    std::map<cgltf_material*, std::string> material_helper;
    for (int i = 0; i < data->materials_count; ++i)
    {
        //TODO
    }

    // Load gltf mesh
    std::map<cgltf_mesh*, std::string> mesh_helper;
    for (int i = 0; i < data->meshes_count; ++i)
    {
        cgltf_mesh* cmesh = &data->meshes[i];
        std::string asset_path = output_path + "/meshes/" + cmesh->name;

        std::vector<uint8_t> bin;
        std::vector<GltfSubMesh> gltf_sub_meshes;
        for (int i = 0; i < cmesh->primitives_count; i++)
        {
            cgltf_primitive* cprimitive = &cmesh->primitives[i];
            GltfSubMesh gltf_sub_mesh;
            gltf_sub_mesh.total_size = 0;
            gltf_sub_mesh.total_offset = bin.size();

            cgltf_attribute* position_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_position);
            cgltf_accessor* position_accessor = position_attribute->data;
            cgltf_buffer_view* position_view = position_accessor->buffer_view;
            uint32_t vertex_count = (uint32_t)position_accessor->count;
            gltf_sub_mesh.vertex_count = vertex_count;
            gltf_sub_mesh.vertex_size = 0;

            const float* minp = &position_accessor->min[0];
            const float* maxp = &position_accessor->max[0];
            gltf_sub_mesh.bounding_box.merge(glm::vec3(minp[0], minp[1], minp[2]));
            gltf_sub_mesh.bounding_box.merge(glm::vec3(maxp[0], maxp[1], maxp[2]));

            uint8_t* position_data = (uint8_t*)(position_view->buffer->data) + position_accessor->offset + position_view->offset;
            gltf_sub_mesh.vertex_size += 3 * sizeof(float);
            gltf_sub_mesh.position_size = vertex_count * 3 * sizeof(float);
            gltf_sub_mesh.position_offset = bin.size();
            gltf_sub_mesh.total_size += gltf_sub_mesh.position_size;
            bin.insert(bin.end(), position_data, position_data + vertex_count * 3 * sizeof(float));

            cgltf_attribute* normal_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_normal);
            cgltf_accessor* normal_accessor = normal_attribute->data;
            cgltf_buffer_view* normal_view = normal_accessor->buffer_view;
            uint8_t* normal_data = (uint8_t*)(normal_view->buffer->data) + normal_accessor->offset + normal_view->offset;
            gltf_sub_mesh.vertex_size += 3 * sizeof(float);
            gltf_sub_mesh.normal_size = vertex_count * 3 * sizeof(float);
            gltf_sub_mesh.normal_offset = bin.size();
            gltf_sub_mesh.total_size += gltf_sub_mesh.normal_size;
            bin.insert(bin.end(), normal_data, normal_data + vertex_count * 3 * sizeof(float));

            cgltf_attribute* tangent_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_tangent);
            cgltf_accessor* tangent_accessor = tangent_attribute->data;
            cgltf_buffer_view* tangent_view = tangent_accessor->buffer_view;
            uint8_t* tangent_data = (uint8_t*)(tangent_view->buffer->data) + tangent_accessor->offset + tangent_view->offset;
            gltf_sub_mesh.vertex_size += 3 * sizeof(float);
            gltf_sub_mesh.tangent_size = vertex_count * 3 * sizeof(float);
            gltf_sub_mesh.tangent_offset = bin.size();
            gltf_sub_mesh.total_size += gltf_sub_mesh.tangent_size;
            bin.insert(bin.end(), tangent_data, tangent_data + vertex_count * 3 * sizeof(float));

            cgltf_attribute* texcoord_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_texcoord);
            cgltf_accessor* texcoord_accessor = texcoord_attribute->data;
            cgltf_buffer_view* texcoord_view = texcoord_accessor->buffer_view;
            uint8_t* uv_data = (uint8_t*)(texcoord_view->buffer->data) + texcoord_accessor->offset + texcoord_view->offset;
            gltf_sub_mesh.vertex_size += 2 * sizeof(float);
            gltf_sub_mesh.uv_size = vertex_count * 2 * sizeof(float);
            gltf_sub_mesh.uv_offset = bin.size();
            gltf_sub_mesh.total_size += gltf_sub_mesh.uv_size;
            bin.insert(bin.end(), uv_data, uv_data + vertex_count * 2 * sizeof(float));

            cgltf_accessor* index_accessor = cprimitive->indices;
            cgltf_buffer_view* index_buffer_view = index_accessor->buffer_view;
            cgltf_buffer* index_buffer = index_buffer_view->buffer;
            uint8_t* index_data = (uint8_t*)index_buffer->data + index_accessor->offset + index_buffer_view->offset;
            uint32_t index_count = (uint32_t)index_accessor->count;
            gltf_sub_mesh.index_count = index_count;
            if (index_accessor->component_type == cgltf_component_type_r_16u)
            {
                gltf_sub_mesh.using_16u_index = true;
                gltf_sub_mesh.index_size = index_count * sizeof(uint16_t);
                gltf_sub_mesh.index_offset = bin.size();
                gltf_sub_mesh.total_size += gltf_sub_mesh.index_size;
                bin.insert(bin.end(), index_data, index_data + index_count * sizeof(uint16_t));
            }
            else
            {
                gltf_sub_mesh.using_16u_index = false;
                gltf_sub_mesh.index_size = index_count * sizeof(uint32_t);
                gltf_sub_mesh.index_offset = bin.size();
                gltf_sub_mesh.total_size += gltf_sub_mesh.index_size;
                bin.insert(bin.end(), index_data, index_data + index_count * sizeof(uint32_t));
            }

            gltf_sub_meshes.push_back(gltf_sub_mesh);
        }

        // Gen json
        rapidjson::Document doc;
        rapidjson::StringBuffer str_buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buffer);
        writer.StartObject();
        writer.Key("data_size");
        writer.Int(bin.size());

        writer.Key("sub_meshes");
        writer.StartArray();
        for (int j = 0; j < gltf_sub_meshes.size(); ++j)
        {
            GltfSubMesh& gltf_sub_mesh = gltf_sub_meshes[j];
            writer.StartObject();
            writer.Key("total_size");
            writer.Int(gltf_sub_mesh.total_size);
            writer.Key("total_offset");
            writer.Int(gltf_sub_mesh.total_offset);
            writer.Key("vertex_count");
            writer.Int(gltf_sub_mesh.vertex_count);
            writer.Key("vertex_size");
            writer.Int(gltf_sub_mesh.vertex_size);
            writer.Key("position_size");
            writer.Int(gltf_sub_mesh.position_size);
            writer.Key("position_offset");
            writer.Int(gltf_sub_mesh.position_offset);
            writer.Key("normal_size");
            writer.Int(gltf_sub_mesh.normal_size);
            writer.Key("normal_offset");
            writer.Int(gltf_sub_mesh.normal_offset);
            writer.Key("tangent_size");
            writer.Int(gltf_sub_mesh.tangent_size);
            writer.Key("tangent_offset");
            writer.Int(gltf_sub_mesh.tangent_offset);
            writer.Key("uv_size");
            writer.Int(gltf_sub_mesh.uv_size);
            writer.Key("uv_offset");
            writer.Int(gltf_sub_mesh.uv_offset);
            writer.Key("index_count");
            writer.Int(gltf_sub_mesh.index_count);
            writer.Key("index_size");
            writer.Int(gltf_sub_mesh.index_size);
            writer.Key("index_offset");
            writer.Int(gltf_sub_mesh.index_offset);
            writer.Key("using_16u");
            writer.Bool(gltf_sub_mesh.using_16u_index);
            writer.Key("maxp");
            Serialization::w_vec3(writer, gltf_sub_mesh.bounding_box.bb_max);
            writer.Key("minp");
            Serialization::w_vec3(writer, gltf_sub_mesh.bounding_box.bb_min);
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
        doc.Parse(str_buffer.GetString());

        if (!AssetManager::get()->exist_asset(asset_path))
        {
            Mesh* mesh = AssetManager::get()->create<Mesh>(asset_path);
            mesh->deserialize(doc.GetObject(), bin);
            AssetManager::get()->save(mesh);
        }

        mesh_helper[cmesh] = asset_path;
    }

    // Load level
    {
        //TODO
    }

    cgltf_free(data);
}