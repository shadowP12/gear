#pragma once

#include "asset.h"

class Material : public Asset
{
public:
    Material(const std::string& asset_path = "");

    virtual ~Material();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, std::vector<uint8_t>& bin);
    virtual void deserialize(const rapidjson::Value& reader, const std::vector<uint8_t>& bin);
};