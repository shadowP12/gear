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

enum class LightType
{
    Point,
    Spot,
    Direction,
};

struct OmniLightData
{
    glm::vec3 position;
    float inv_radius;

    glm::vec3 direction;
    float size;

    glm::vec3 color;
    float intensity;

    float attenuation;
    float cone_attenuation;
    float cone_angle;
    float pad0;
};

struct DirectionLightData
{
    glm::vec3 direction;
    float size;
    glm::vec3 color;
    float intensity;
};

enum SamplerType
{
    SAMPLER_NEAREST_CLAMP = 0,
    SAMPLER_LINEAR_CLAMP = 1,
    SAMPLER_MAX_COUNT = 2,
};