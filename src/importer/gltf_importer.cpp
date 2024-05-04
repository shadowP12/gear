#include "gltf_importer.h"
#include "asset/asset.h"
#include "asset/texture2d.h"
#include "asset/asset_manager.h"
#include <core/path.h>
#include <core/io/dir_access.h>
#include <map>
#include <string>
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

static char* g_gltf_texture_2d_json =
    "{\n"
    "   \"uri\": \"${URI}\"\n"
    "}\n";

bool json_token_replace(std::string& json_str, const std::string& from, const std::string& to)
{
    size_t start_pos = json_str.find(from);
    if(start_pos == std::string::npos)
        return false;
    json_str.replace(start_pos, from.length(), to);
    return true;
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
            std::string asset_json = g_gltf_texture_2d_json;
            json_token_replace(asset_json, "${URI}", Path::filename(output_image_path));
            rapidjson::Document doc;
            doc.Parse(asset_json.c_str());

            // Gen bin
            std::vector<uint8_t> bin;

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
        //TODO
    }

    // Load gltf mesh
    for (int i = 0; i < data->meshes_count; ++i)
    {
        //TODO
    }

    cgltf_free(data);
}