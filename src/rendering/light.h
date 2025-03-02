#pragma once
#include <glm/glm.hpp>

enum class LightType
{
    Point,
    Spot,
    Direction,
};

struct OmniLight
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

struct DirectionLight
{
    glm::vec3 direction;
    float size;
    glm::vec3 color;
    float intensity;
};
