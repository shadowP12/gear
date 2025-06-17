#pragma once

#include <core/module.h>
#include <string>

class GltfImporter : public Module<GltfImporter>
{
public:
    GltfImporter() = default;
    ~GltfImporter() = default;

    void import_asset(const std::string& file_path, const std::string& output_path);
};