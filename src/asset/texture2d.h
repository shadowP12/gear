#pragma once

#include "asset.h"

class Texture2D : public Asset
{
public:
    Texture2D(const std::string& asset_path = "");

    virtual ~Texture2D();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, std::vector<uint8_t>& bin);
    virtual void deserialize(const rapidjson::Value& reader, const std::vector<uint8_t>& bin);

private:
    std::string _uri;
};