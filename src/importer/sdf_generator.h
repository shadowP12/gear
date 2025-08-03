#pragma once

#include <math/bounding_box.h>

#include <string>

class SDFAsset;

#define DECLARE_SDF_GENERATE(IndexType)             \
    SDFAsset* generate_sdf(const std::string& path, \
                           uint32_t vertex_count,   \
                           float* vertices,         \
                           uint32_t index_count,    \
                           IndexType* indices);

DECLARE_SDF_GENERATE(uint16_t)
DECLARE_SDF_GENERATE(uint32_t)