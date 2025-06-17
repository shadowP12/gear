#pragma once

#include "asset.h"

#include <core/io/dir_access.h>
#include <core/io/file_access.h>
#include <core/module.h>
#include <core/path.h>

#include <memory>
#include <string>
#include <unordered_map>

class AssetManager : public Module<AssetManager>
{
public:
    AssetManager() = default;
    ~AssetManager() = default;

    void setup();

    void finish();

    template<class T>
    T* create(const std::string& asset_path)
    {
        if (has_asset(asset_path))
            return nullptr;

        T* asset = new T(asset_path);
        _asset_dict[asset_path] = asset;

        return (T*)asset;
    }

    template<class T>
    T* load(const std::string& asset_path)
    {
        if (has_asset(asset_path))
            return (T*)get_asset(asset_path);

        std::string json_path = Path::fix_path(asset_path + ".json");
        std::string bin_path = Path::fix_path(asset_path + ".bin");

        if (!FileAccess::exist(json_path))
            return nullptr;

        rapidjson::Document dom;
        {
            std::shared_ptr<FileAccess> fa = std::shared_ptr<FileAccess>(FileAccess::open(json_path, FileAccess::READ));
            std::vector<char> buf;
            uint32_t buf_size = fa->get_length();
            buf.resize(buf_size);
            fa->get_buffer((uint8_t*)buf.data(), buf_size);

            if (dom.Parse(buf.data(), buf_size).HasParseError())
                return nullptr;
        }
        DeserializationContext ctx(&dom);

        BinaryStream bin_stream;
        if (FileAccess::exist(bin_path))
        {
            std::shared_ptr<FileAccess> fa = std::shared_ptr<FileAccess>(FileAccess::open(bin_path, FileAccess::READ));
            uint32_t bin_size = fa->get_length();
            std::vector<uint8_t> bin_data(bin_size);
            fa->get_buffer(bin_data.data(), bin_size);
            bin_stream = std::move(bin_data);
        }

        T* asset = create<T>(asset_path);
        asset->deserialize(ctx, bin_stream);

        return asset;
    }

    void save(Asset* asset);

    bool exist_asset(const std::string& asset_path);

    bool has_asset(const std::string& asset_path);

    Asset* get_asset(const std::string& asset_path);

private:
    std::unordered_map<std::string, Asset*> _asset_dict;
};