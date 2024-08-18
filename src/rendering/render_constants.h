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

struct RenderView
{
    enum Type
    {
        VIEW_TYPE_MAIN,
        VIEW_TYPE_DISPLAY,
        VIEW_TYPE_MAX,
    };

    float zn;
    float zf;
    float ev100;
    float exposure;
    glm::vec3 position;
    glm::vec3 view_direction;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    bool is_orthogonal;
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