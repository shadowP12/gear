#pragma once

#include <glm/glm.hpp>

enum class MaterialAlphaMode
{
    Opaque = 0,
    Mask = 1,
    Blend = 2,
};

struct MaterialParams
{
    glm::vec4 base_color;
};