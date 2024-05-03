#include "asset_manager.h"

AssetManager::AssetManager()
{}

AssetManager::~AssetManager()
{}

void AssetManager::save(Asset* asset)
{
    std::vector<uint8_t> bin;
    rapidjson::StringBuffer str_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(str_buffer);

    writer.StartObject();
    asset->serialize(writer, bin);
    writer.EndObject();

    {
        std::string json_path = asset->get_asset_path() + ".json";
        std::shared_ptr<FileAccess> fa = std::shared_ptr<FileAccess>(FileAccess::open(json_path, FileAccess::WRITE));
        fa->store_string(str_buffer.GetString());
    }

    if (!bin.empty())
    {
        std::string bin_path = asset->get_asset_path() + ".bin";
        std::shared_ptr<FileAccess> fa = std::shared_ptr<FileAccess>(FileAccess::open(bin_path, FileAccess::WRITE));
        fa->store_buffer(bin.data(), bin.size());
    }
}