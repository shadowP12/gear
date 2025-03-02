#pragma once

#include <glm/glm.hpp>

struct FrameConstants
{
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
};

struct SceneInstanceData
{
    glm::mat4 transform;
    glm::mat4 pad0;
    glm::mat4 pad1;
    glm::mat4 pad2;
};

enum SamplerType
{
    SAMPLER_NEAREST_CLAMP = 0,
    SAMPLER_LINEAR_CLAMP = 1,
    SAMPLER_MAX_COUNT = 2,
};