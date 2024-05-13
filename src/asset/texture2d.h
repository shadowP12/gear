#pragma once

#include "asset.h"

class Texture2D : public Asset
{
public:
    Texture2D(const std::string& asset_path = "");

    virtual ~Texture2D();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);
    virtual void deserialize(const rapidjson::Value& value, Serialization::BinaryStream& bin);

private:
    std::string _uri;
};