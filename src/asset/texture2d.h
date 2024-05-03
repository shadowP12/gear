#pragma once

#include "asset.h"

class Texture2D : public Asset
{
public:
    Texture2D(const std::string& asset_path = "");

    virtual ~Texture2D();
};