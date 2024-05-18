#include "gltf_importer.h"
#include "asset/asset.h"
#include "asset/texture2d.h"
#include "asset/material.h"
#include "asset/mesh.h"
#include "asset/level.h"
#include "asset/asset_manager.h"
#include <core/path.h>
#include <core/io/dir_access.h>
#include <math/bounding_box.h>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
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
    std::string material_path;
};

struct GltfNode
{
    std::string name;
    glm::vec3 translation;
    glm::vec3 scale;
    glm::vec3 euler;
    bool has_parent;
    cgltf_node* parent_node;
    bool has_mesh;
    std::string mesh_path;
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
        std::string asset_path = "asset://" + output_path + "/textures/" + Path::filename(cimage->uri);
        std::string output_image_path = Path::fix_path(asset_path);

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

            Serialization::BinaryStream bin;
            Texture2D* tex_2d = AssetManager::get()->create<Texture2D>(asset_path);
            tex_2d->deserialize(doc.GetObject(), bin);
            AssetManager::get()->save(tex_2d);
        }

        image_helper[cimage] = asset_path;
    }

    // Load gltf material
    std::map<cgltf_material*, std::string> material_helper;
    for (int i = 0; i < data->materials_count; ++i)
    {
        cgltf_material* cmaterial = &data->materials[i];
        std::string asset_path = "asset://" + output_path + "/materials/" + cmaterial->name;

        if (!AssetManager::get()->exist_asset(asset_path))
        {
            rapidjson::Document doc;
            rapidjson::StringBuffer str_buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buffer);
            writer.StartObject();

            writer.Key("alpha_mode");
            if (cmaterial->alpha_mode == cgltf_alpha_mode_opaque) {
                writer.Int(0);
            } else if (cmaterial->alpha_mode == cgltf_alpha_mode_mask) {
                writer.Int(1);
            } else {
                writer.Int(2);
            }

            if (cmaterial->has_pbr_metallic_roughness)
            {
                glm::vec4 base_color = glm::make_vec4(cmaterial->pbr_metallic_roughness.base_color_factor);
                writer.Key("base_color");
                Serialization::w_vec4(writer, base_color);

                if(cmaterial->pbr_metallic_roughness.base_color_texture.texture)
                {
                    cgltf_image* cimage = cmaterial->pbr_metallic_roughness.base_color_texture.texture->image;
                    cgltf_sampler* csampler = cmaterial->pbr_metallic_roughness.base_color_texture.texture->sampler;
                    writer.Key("base_color_texture");
                    writer.String(image_helper[cimage].c_str());
                }
            }
            else
            {
                glm::vec4 base_color = glm::make_vec4(cmaterial->pbr_specular_glossiness.diffuse_factor);
                writer.Key("base_color");
                Serialization::w_vec4(writer, base_color);

                if(cmaterial->pbr_specular_glossiness.diffuse_texture.texture)
                {
                    cgltf_image* cimage = cmaterial->pbr_specular_glossiness.diffuse_texture.texture->image;
                    cgltf_sampler* csampler = cmaterial->pbr_specular_glossiness.diffuse_texture.texture->sampler;
                    writer.Key("base_color_texture");
                    writer.String(image_helper[cimage].c_str());
                }
            }

            writer.EndObject();
            doc.Parse(str_buffer.GetString());

            Serialization::BinaryStream bin;
            Material* mat = AssetManager::get()->create<Material>(asset_path);
            mat->deserialize(doc.GetObject(), bin);
            AssetManager::get()->save(mat);
        }
        material_helper[cmaterial] = asset_path;
    }

    // Load gltf mesh
    std::map<cgltf_mesh*, std::string> mesh_helper;
    for (int i = 0; i < data->meshes_count; ++i)
    {
        cgltf_mesh* cmesh = &data->meshes[i];
        std::string asset_path = "asset://" + output_path + "/meshes/" + cmesh->name;

        Serialization::BinaryStream bin;
        std::vector<GltfSubMesh> gltf_sub_meshes;
        for (int i = 0; i < cmesh->primitives_count; i++)
        {
            cgltf_primitive* cprimitive = &cmesh->primitives[i];
            cgltf_material* cmaterial = cprimitive->material;
            GltfSubMesh gltf_sub_mesh;
            gltf_sub_mesh.material_path = material_helper[cmaterial];
            gltf_sub_mesh.total_size = 0;
            gltf_sub_mesh.total_offset = bin.get_size();

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
            gltf_sub_mesh.position_offset = bin.get_size();
            gltf_sub_mesh.total_size += gltf_sub_mesh.position_size;
            bin.write(position_data, gltf_sub_mesh.position_size);

            cgltf_attribute* normal_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_normal);
            cgltf_accessor* normal_accessor = normal_attribute->data;
            cgltf_buffer_view* normal_view = normal_accessor->buffer_view;
            uint8_t* normal_data = (uint8_t*)(normal_view->buffer->data) + normal_accessor->offset + normal_view->offset;
            gltf_sub_mesh.vertex_size += 3 * sizeof(float);
            gltf_sub_mesh.normal_size = vertex_count * 3 * sizeof(float);
            gltf_sub_mesh.normal_offset = bin.get_size();
            gltf_sub_mesh.total_size += gltf_sub_mesh.normal_size;
            bin.write(normal_data, gltf_sub_mesh.normal_size);

            cgltf_attribute* tangent_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_tangent);
            cgltf_accessor* tangent_accessor = tangent_attribute->data;
            cgltf_buffer_view* tangent_view = tangent_accessor->buffer_view;
            uint8_t* tangent_data = (uint8_t*)(tangent_view->buffer->data) + tangent_accessor->offset + tangent_view->offset;
            gltf_sub_mesh.vertex_size += 3 * sizeof(float);
            gltf_sub_mesh.tangent_size = vertex_count * 3 * sizeof(float);
            gltf_sub_mesh.tangent_offset = bin.get_size();
            gltf_sub_mesh.total_size += gltf_sub_mesh.tangent_size;
            bin.write(tangent_data, gltf_sub_mesh.tangent_size);

            cgltf_attribute* texcoord_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_texcoord);
            cgltf_accessor* texcoord_accessor = texcoord_attribute->data;
            cgltf_buffer_view* texcoord_view = texcoord_accessor->buffer_view;
            uint8_t* uv_data = (uint8_t*)(texcoord_view->buffer->data) + texcoord_accessor->offset + texcoord_view->offset;
            gltf_sub_mesh.vertex_size += 2 * sizeof(float);
            gltf_sub_mesh.uv_size = vertex_count * 2 * sizeof(float);
            gltf_sub_mesh.uv_offset = bin.get_size();
            gltf_sub_mesh.total_size += gltf_sub_mesh.uv_size;
            bin.write(uv_data, gltf_sub_mesh.uv_size);

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
                gltf_sub_mesh.index_offset = bin.get_size();
                gltf_sub_mesh.total_size += gltf_sub_mesh.index_size;
                bin.write(index_data, gltf_sub_mesh.index_size);
            }
            else
            {
                gltf_sub_mesh.using_16u_index = false;
                gltf_sub_mesh.index_size = index_count * sizeof(uint32_t);
                gltf_sub_mesh.index_offset = bin.get_size();
                gltf_sub_mesh.total_size += gltf_sub_mesh.index_size;
                bin.write(index_data, gltf_sub_mesh.index_size);
            }

            gltf_sub_meshes.push_back(gltf_sub_mesh);
        }

        // Gen json
        rapidjson::Document doc;
        rapidjson::StringBuffer str_buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buffer);
        writer.StartObject();
        writer.Key("data_size");
        writer.Int(bin.get_size());

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
            writer.Key("material");
            writer.String(gltf_sub_mesh.material_path.c_str());
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
        std::vector<GltfNode> gltf_nodes;
        gltf_nodes.resize(data->nodes_count);
        std::map<cgltf_node*, int> node_helper;
        for (size_t i = 0; i < data->nodes_count; ++i)
        {
            cgltf_node* cnode = &data->nodes[i];
            node_helper[cnode] = i;

            GltfNode gltf_node;
            gltf_node.name = cnode->name;
            gltf_node.has_parent = cnode->parent != nullptr;
            gltf_node.parent_node = cnode->parent;
            gltf_node.has_mesh = cnode->mesh != nullptr;
            if (gltf_node.has_mesh)
            {
                gltf_node.mesh_path = mesh_helper[cnode->mesh];
            }

            glm::vec3 translation = glm::vec3(0.0f);
            if (cnode->has_translation)
            {
                translation.x = cnode->translation[0];
                translation.y = cnode->translation[1];
                translation.z = cnode->translation[2];
            }

            glm::quat rotation = glm::quat(1, 0, 0, 0);
            if (cnode->has_rotation)
            {
                rotation.x = cnode->rotation[0];
                rotation.y = cnode->rotation[1];
                rotation.z = cnode->rotation[2];
                rotation.w = cnode->rotation[3];
            }

            glm::vec3 scale = glm::vec3(1.0f);
            if (cnode->has_scale)
            {
                scale.x = cnode->scale[0];
                scale.y = cnode->scale[1];
                scale.z = cnode->scale[2];
            }
            glm::vec3 euler = glm::eulerAngles(rotation) * 3.14159f / 180.f;

            gltf_node.translation = translation;
            gltf_node.scale = scale;
            gltf_node.euler = euler;
            gltf_nodes.push_back(gltf_node);
        }

        rapidjson::Document doc;
        rapidjson::StringBuffer str_buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buffer);
        writer.StartObject();
        writer.Key("hierarchy");
        writer.StartArray();
        for (size_t i = 0; i < gltf_nodes.size(); ++i)
        {
            if (gltf_nodes[i].has_parent)
                writer.Int(node_helper[gltf_nodes[i].parent_node]);
            else
                writer.Int(-1);
        }
        writer.EndArray();

        writer.Key("entities");
        writer.StartArray();
        for (size_t i = 0; i < gltf_nodes.size(); ++i)
        {
            GltfNode& gltf_node = gltf_nodes[i];
            writer.StartObject();
            writer.Key("name");
            writer.String(gltf_node.name.c_str());
            writer.Key("components");
            writer.StartArray();

            writer.StartObject();
            writer.Key("class_name");
            writer.String("CTransform");
            writer.Key("translation");
            Serialization::w_vec3(writer, gltf_node.translation);
            writer.Key("scale");
            Serialization::w_vec3(writer, gltf_node.scale);
            writer.Key("euler");
            Serialization::w_vec3(writer, gltf_node.euler);
            writer.EndObject();

            if (gltf_node.has_mesh)
            {
                writer.StartObject();
                writer.Key("class_name");
                writer.String("CMesh");
                writer.Key("mesh");
                writer.String(gltf_node.mesh_path.c_str());
                writer.EndObject();
            }
            writer.EndArray();
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
        doc.Parse(str_buffer.GetString());

        std::string asset_path = "asset://" + output_path + "/" + data->scene->name;
        if (!AssetManager::get()->exist_asset(asset_path))
        {
            Serialization::BinaryStream bin;
            Level* level = AssetManager::get()->create<Level>(asset_path);
            level->deserialize(doc.GetObject(), bin);
            AssetManager::get()->save(level);
        }
    }

    cgltf_free(data);
}