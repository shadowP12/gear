#include "asset_manager.h"

void AssetManager::setup()
{
}

void AssetManager::clear()
{
    for (auto& iter : _asset_dict) {
        delete iter.second;
    }
    _asset_dict.clear();
}

void AssetManager::save(Asset* asset)
{
    std::vector<uint8_t> bin;
    rapidjson::StringBuffer str_buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(str_buffer);

    asset->serialize(writer, bin);

    {
        std::string json_path = asset->get_asset_path() + ".json";
        if (!DirAccess::dir_exists(Path::parent_path(json_path))) {
            DirAccess::make_dir_recursive(Path::parent_path(json_path));
        }

        std::shared_ptr<FileAccess> fa = std::shared_ptr<FileAccess>(FileAccess::open(json_path, FileAccess::WRITE));
        fa->store_string(str_buffer.GetString());
    }

    if (!bin.empty())
    {
        std::string bin_path = asset->get_asset_path() + ".bin";
        if (!DirAccess::dir_exists(Path::parent_path(bin_path))) {
            DirAccess::make_dir_recursive(Path::parent_path(bin_path));
        }

        std::shared_ptr<FileAccess> fa = std::shared_ptr<FileAccess>(FileAccess::open(bin_path, FileAccess::WRITE));
        fa->store_buffer(bin.data(), bin.size());
    }
}

bool AssetManager::exist_asset(const std::string& asset_path)
{
    std::string json_path = asset_path + ".json";
    if (FileAccess::exist(json_path)) {
        return true;
    }
    return false;
}

bool AssetManager::has_asset(const std::string& asset_path)
{
    auto iter = _asset_dict.find(asset_path);
    if (iter != _asset_dict.end()) {
        return true;
    }
    return false;
}

Asset* AssetManager::get_asset(const std::string& asset_path)
{
    auto iter = _asset_dict.find(asset_path);
    if (iter != _asset_dict.end()) {
        return iter->second;
    }
    return nullptr;
}