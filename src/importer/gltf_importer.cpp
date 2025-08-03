#include "gltf_importer.h"
#include "asset/asset.h"
#include "asset/asset_manager.h"
#include "asset/level.h"
#include "asset/material_asset.h"
#include "asset/mesh_asset.h"
#include "asset/texture_asset.h"
#include "rendering/vertex_factory.h"
#include "sdf_generator.h"

#include <core/io/dir_access.h>
#include <core/path.h>
#include <math/bounding_box.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <map>
#include <string>

struct GltfPrimitive {
    uint32_t total_size;
    uint32_t total_offset;
    uint32_t vertex_count;
    uint32_t vertex_size;
    uint32_t vertex_offset;
    uint32_t position_size;
    uint32_t position_offset;
    uint32_t normal_size;
    uint32_t normal_offset;
    uint32_t tangent_size;
    uint32_t tangent_offset;
    uint32_t uv0_size;
    uint32_t uv0_offset;
    uint32_t uv1_size;
    uint32_t uv1_offset;
    uint32_t index_count;
    uint32_t index_size;
    uint32_t index_offset;
    bool using_16u_index;
    BoundingBox bounding_box;
    std::string material_path;
    std::string sdf_path;
    cgltf_primitive_type primitive_type;
};

struct GltfNode {
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
        if (att->type == type)
            return att;
    }
    return nullptr;
}

VkPrimitiveTopology get_primitive_topology_from_gltf(cgltf_primitive_type gltf_primitive_type)
{
    switch (gltf_primitive_type)
    {
        case cgltf_primitive_type_points:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case cgltf_primitive_type_lines:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case cgltf_primitive_type_line_strip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case cgltf_primitive_type_triangles:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case cgltf_primitive_type_triangle_strip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case cgltf_primitive_type_triangle_fan:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    }
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
}

MaterialAsset* import_material(const std::string& path, cgltf_material* cmaterial)
{
    rapidjson::Document doc;
    rapidjson::StringBuffer str_buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buffer);
    SerializationContext s_ctx(&writer);
    s_ctx.object([&]() {
        s_ctx.array("passes", [&]() {
            s_ctx.object([&]() {
                s_ctx.field("draw_type", DRAW_OPAQUE);
                s_ctx.field("vs", std::string("shader://default.vert"));
                s_ctx.field("fs", std::string("shader://default.frag"));
            });

            s_ctx.object([&]() {
                s_ctx.field("draw_type", DRAW_SHADOW);
                s_ctx.field("vs", std::string("shader://shadow.vert"));
                s_ctx.field("fs", std::string("shader://shadow.frag"));
            });
        });
    });
    doc.Parse(str_buffer.GetString());
    DeserializationContext d_ctx(&doc);

    MaterialAsset* mat = AssetManager::get()->create<MaterialAsset>(path);
    BinaryStream empty_bin_stream;
    mat->deserialize(d_ctx, empty_bin_stream);
    AssetManager::get()->save(mat);
    return mat;
}

void GltfImporter::import_asset(const std::string& file_path, const std::string& output_path)
{
    cgltf_options options = {static_cast<cgltf_file_type>(0)};
    cgltf_data* data = nullptr;
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
            if (!FileAccess::exist(output_image_path))
            {
                if (!DirAccess::dir_exists(Path::parent_path(output_image_path)))
                {
                    DirAccess::make_dir_recursive(Path::parent_path(output_image_path));
                }
                DirAccess::copy(input_image_path, output_image_path);
            }

            // Gen json
            rapidjson::Document doc;
            rapidjson::StringBuffer str_buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buffer);
            SerializationContext s_ctx(&writer);
            s_ctx.object([&]() {
                s_ctx.field("uri", asset_path);
            });
            doc.Parse(str_buffer.GetString());
            DeserializationContext d_ctx(&doc);

            BinaryStream bin_stream;
            TextureAsset* tex_2d = AssetManager::get()->create<TextureAsset>(asset_path);
            tex_2d->deserialize(d_ctx, bin_stream);
            AssetManager::get()->save(tex_2d);
        }

        image_helper[cimage] = asset_path;
    }

    // Load gltf mesh
    std::map<cgltf_mesh*, std::string> mesh_helper;
    for (int i = 0; i < data->meshes_count; ++i)
    {
        cgltf_mesh* cmesh = &data->meshes[i];
        std::string asset_path = "asset://" + output_path + "/meshes/" + cmesh->name;
        if (!AssetManager::get()->exist_asset(asset_path))
        {
            BinaryStream bin_stream;
            std::vector<GltfPrimitive> gltf_primitives;
            for (int i = 0; i < cmesh->primitives_count; i++)
            {
                cgltf_primitive* cprimitive = &cmesh->primitives[i];
                cgltf_material* cmaterial = cprimitive->material;

                GltfPrimitive gltf_primitive;
                gltf_primitive.primitive_type = cprimitive->type;
                gltf_primitive.total_size = 0;
                gltf_primitive.total_offset = bin_stream.get_size();

                cgltf_attribute* position_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_position);
                cgltf_accessor* position_accessor = position_attribute->data;
                cgltf_buffer_view* position_view = position_accessor->buffer_view;
                uint32_t vertex_count = (uint32_t)position_accessor->count;
                gltf_primitive.vertex_count = vertex_count;
                gltf_primitive.vertex_size = 0;
                gltf_primitive.vertex_offset = bin_stream.get_size();

                const float* minp = &position_accessor->min[0];
                const float* maxp = &position_accessor->max[0];
                gltf_primitive.bounding_box.merge(glm::vec3(minp[0], minp[1], minp[2]));
                gltf_primitive.bounding_box.merge(glm::vec3(maxp[0], maxp[1], maxp[2]));

                uint8_t* position_data = (uint8_t*)(position_view->buffer->data) + position_accessor->offset + position_view->offset;
                gltf_primitive.position_size = vertex_count * 3 * sizeof(float);
                gltf_primitive.position_offset = bin_stream.get_size();
                gltf_primitive.vertex_size += gltf_primitive.position_size;
                gltf_primitive.total_size += gltf_primitive.position_size;
                bin_stream.write(position_data, gltf_primitive.position_size);

                cgltf_attribute* normal_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_normal);
                cgltf_accessor* normal_accessor = normal_attribute->data;
                cgltf_buffer_view* normal_view = normal_accessor->buffer_view;
                uint8_t* normal_data = (uint8_t*)(normal_view->buffer->data) + normal_accessor->offset + normal_view->offset;
                gltf_primitive.normal_size = vertex_count * 3 * sizeof(float);
                gltf_primitive.normal_offset = bin_stream.get_size();
                gltf_primitive.vertex_size += gltf_primitive.normal_size;
                gltf_primitive.total_size += gltf_primitive.normal_size;
                bin_stream.write(normal_data, gltf_primitive.normal_size);

                cgltf_attribute* tangent_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_tangent);
                cgltf_accessor* tangent_accessor = tangent_attribute->data;
                cgltf_buffer_view* tangent_view = tangent_accessor->buffer_view;
                uint8_t* tangent_data = (uint8_t*)(tangent_view->buffer->data) + tangent_accessor->offset + tangent_view->offset;
                gltf_primitive.tangent_size = vertex_count * 3 * sizeof(float);
                gltf_primitive.tangent_offset = bin_stream.get_size();
                gltf_primitive.vertex_size += gltf_primitive.tangent_size;
                gltf_primitive.total_size += gltf_primitive.tangent_size;
                bin_stream.write(tangent_data, gltf_primitive.tangent_size);

                cgltf_attribute* texcoord_attribute = get_gltf_attribute(cprimitive, cgltf_attribute_type_texcoord);
                cgltf_accessor* texcoord_accessor = texcoord_attribute->data;
                cgltf_buffer_view* texcoord_view = texcoord_accessor->buffer_view;
                uint8_t* uv_data = (uint8_t*)(texcoord_view->buffer->data) + texcoord_accessor->offset + texcoord_view->offset;
                gltf_primitive.uv0_size = vertex_count * 2 * sizeof(float);
                gltf_primitive.uv0_offset = bin_stream.get_size();
                gltf_primitive.vertex_size += gltf_primitive.uv0_size;
                gltf_primitive.total_size += gltf_primitive.uv0_size;
                bin_stream.write(uv_data, gltf_primitive.uv0_size);

                gltf_primitive.uv1_size = vertex_count * 2 * sizeof(float);
                gltf_primitive.uv1_offset = bin_stream.get_size();
                gltf_primitive.vertex_size += gltf_primitive.uv1_size;
                gltf_primitive.total_size += gltf_primitive.uv1_size;
                bin_stream.write(uv_data, gltf_primitive.uv1_size);

                cgltf_accessor* index_accessor = cprimitive->indices;
                cgltf_buffer_view* index_buffer_view = index_accessor->buffer_view;
                cgltf_buffer* index_buffer = index_buffer_view->buffer;
                uint8_t* index_data = (uint8_t*)index_buffer->data + index_accessor->offset + index_buffer_view->offset;
                uint32_t index_count = (uint32_t)index_accessor->count;
                bool using_16u_index = index_accessor->component_type == cgltf_component_type_r_16u;
                uint32_t* indices_32 = using_16u_index ? nullptr : (uint32_t*)index_data;
                uint16_t* indices_16 = using_16u_index ? (uint16_t*)index_data : nullptr;

                gltf_primitive.using_16u_index = using_16u_index;
                gltf_primitive.index_count = index_count;
                gltf_primitive.index_size = using_16u_index ? index_count * sizeof(uint16_t) : index_count * sizeof(uint32_t);
                gltf_primitive.index_offset = bin_stream.get_size();
                gltf_primitive.total_size += gltf_primitive.index_size;
                bin_stream.write(index_data, gltf_primitive.index_size);

                // Load gltf material
                std::string material_path = "asset://" + output_path + "/materials/" + cmaterial->name;
                gltf_primitive.material_path = material_path;
                if (!AssetManager::get()->exist_asset(material_path))
                {
                    import_material(material_path, cmaterial);
                }

                // Load sdf
                if(false)
                {
                    std::string sdf_name = std::string(cmesh->name) + "/_" + std::to_string(i);
                    std::string sdf_path = "asset://" + output_path + "/sdfs/" + sdf_name;
                    gltf_primitive.sdf_path = sdf_path;

                    if (!AssetManager::get()->exist_asset(sdf_path))
                    {
                        if (using_16u_index)
                        {
                            generate_sdf(sdf_path, vertex_count, (float*)position_data, index_count, indices_16);
                        }
                        else
                        {
                            generate_sdf(sdf_path, vertex_count, (float*)position_data, index_count, indices_32);
                        }
                    }
                }

                gltf_primitives.push_back(gltf_primitive);
            }

            // Gen json
            rapidjson::Document doc;
            rapidjson::StringBuffer str_buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buffer);
            SerializationContext s_ctx(&writer);
            s_ctx.object([&]() {
                s_ctx.field("data_size", bin_stream.get_size());
                s_ctx.array("surfaces", [&]() {
                    for (auto& gltf_primitive : gltf_primitives)
                    {
                        s_ctx.object([&]() {
                            s_ctx.field("total_size", gltf_primitive.total_size);
                            s_ctx.field("total_offset", gltf_primitive.total_offset);
                            s_ctx.field("vertex_count", gltf_primitive.vertex_count);
                            s_ctx.field("vertex_size", gltf_primitive.vertex_size);
                            s_ctx.field("vertex_offset", gltf_primitive.vertex_offset);
                            s_ctx.field("position_size", gltf_primitive.position_size);
                            s_ctx.field("position_offset", gltf_primitive.position_offset);
                            s_ctx.field("normal_size", gltf_primitive.normal_size);
                            s_ctx.field("normal_offset", gltf_primitive.normal_offset);
                            s_ctx.field("tangent_size", gltf_primitive.tangent_size);
                            s_ctx.field("tangent_offset", gltf_primitive.tangent_offset);
                            s_ctx.field("uv0_size", gltf_primitive.uv0_size);
                            s_ctx.field("uv0_offset", gltf_primitive.uv0_offset);
                            s_ctx.field("uv1_size", gltf_primitive.uv1_size);
                            s_ctx.field("uv1_offset", gltf_primitive.uv1_offset);
                            s_ctx.field("index_count", gltf_primitive.index_count);
                            s_ctx.field("index_size", gltf_primitive.index_size);
                            s_ctx.field("index_offset", gltf_primitive.index_offset);
                            s_ctx.field("using_16u", gltf_primitive.using_16u_index);
                            s_ctx.field("primitive_topology", get_primitive_topology_from_gltf(gltf_primitive.primitive_type));
                            s_ctx.field("bounds", gltf_primitive.bounding_box);
                            s_ctx.field("material", gltf_primitive.material_path);
                            s_ctx.field("sdf", gltf_primitive.sdf_path);
                        });
                    }
                });
            });
            doc.Parse(str_buffer.GetString());
            DeserializationContext d_ctx(&doc);

            MeshAsset* mesh = AssetManager::get()->create<MeshAsset>(asset_path);
            mesh->deserialize(d_ctx, bin_stream);
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
        SerializationContext s_ctx(&writer);
        s_ctx.object([&]() {
            s_ctx.array("hierarchy", [&]() {
                for (size_t i = 0; i < gltf_nodes.size(); ++i)
                {
                    if (gltf_nodes[i].has_parent)
                    {
                        s_ctx.array("hierarchy", [&]() {
                            s_ctx.field(i);
                            s_ctx.field(node_helper[gltf_nodes[i].parent_node]);
                        });
                    }
                }
            });

            s_ctx.array("entities", [&]() {
                for (auto& gltf_node : gltf_nodes)
                {
                    s_ctx.object([&]() {
                        s_ctx.field("name", gltf_node.name);
                        s_ctx.field("translation", gltf_node.translation);
                        s_ctx.field("scale", gltf_node.scale);
                        s_ctx.field("euler", gltf_node.euler);

                        s_ctx.array("components", [&]() {
                            if (gltf_node.has_mesh)
                            {
                                s_ctx.object([&]() {
                                    s_ctx.field("class_name", std::string("CMesh"));
                                    s_ctx.field("mesh", gltf_node.mesh_path);
                                });
                            }
                        });
                    });
                }
            });
        });
        doc.Parse(str_buffer.GetString());
        DeserializationContext d_ctx(&doc);

        std::string asset_path = "asset://" + output_path + "/" + data->scene->name;
        if (!AssetManager::get()->exist_asset(asset_path))
        {
            BinaryStream bin_stream;
            Level* level = AssetManager::get()->create<Level>(asset_path);
            level->deserialize(d_ctx, bin_stream);
            AssetManager::get()->save(level);
        }
    }

    cgltf_free(data);
}