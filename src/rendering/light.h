#pragma once

#include <glm/glm.hpp>

enum class LightType
{
    Point,
    Spot,
    Direction,
};

struct PunctualLight
{
    glm::vec3 position;
    float inv_radius;

    glm::vec3 direction;
    float size;

    glm::vec3 color;
    float intensity;

    float cone_angle;
    float inner_angle;
    float pad0;
    float pad1;
};

struct DirectionLight
{
    glm::vec3 direction;
    float size;
    glm::vec3 color;
    float intensity;
};
