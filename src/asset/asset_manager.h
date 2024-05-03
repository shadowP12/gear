#pragma once

#include "asset.h"
#include <string>
#include <memory>
#include <core/module.h>
#include <core/io/file_access.h>

class AssetManager : public Module<AssetManager>
{
public:
    AssetManager();

    ~AssetManager();

    template<class T>
    T* load(const std::string& asset_path)
    {
        std::string json_path = asset_path + ".json";
        std::string bin_path = asset_path + ".bin";

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

        std::vector<uint8_t> bin;
        if (FileAccess::exist(bin_path))
        {
            std::shared_ptr<FileAccess> fa = std::shared_ptr<FileAccess>(FileAccess::open(bin_path, FileAccess::READ));
            uint32_t bin_size;
            bin_size = fa->get_length();
            bin.resize(bin_size);
            fa->get_buffer(bin.data(), bin_size);
        }

        T* asset = T(asset_path);
        asset->deserialize(dom.GetObject(), bin);

        return asset;
    }

    void save(Asset* asset);
};