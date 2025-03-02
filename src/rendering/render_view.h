#pragma once
#include <glm/glm.hpp>
#include <core/enum_flag.h>

enum class ProjectionMode
{
    Perspective = 0,
    Ortho = 1
};

enum ViewUsageFlags
{
    Main = 0x1,
    Display = 0x2
};
SP_MAKE_ENUM_FLAG(uint32_t, ViewUsageFlags)

struct RenderView
{
    float zn;
    float zf;
    float ev100;
    float exposure;
    glm::vec3 position;
    glm::vec3 direction;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    ProjectionMode proj_model;
};