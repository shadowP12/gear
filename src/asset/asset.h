#pragma once

#include <serialization/serializable.h>

#include <string>

class Asset : public Serializable
{
public:
    Asset(const std::string& asset_path = "");

    virtual ~Asset();

    std::string get_asset_path() { return _asset_path; }

protected:
    std::string _asset_path;
};