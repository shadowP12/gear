#pragma once

#include <string>
#include <core/module.h>

class GltfImporter : public Module<GltfImporter>
{
public:
    GltfImporter() = default;
    ~GltfImporter() = default;

    void import_asset(const std::string& file_path, const std::string& output_path);
};