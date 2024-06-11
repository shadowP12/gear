#pragma once

#include <glm/glm.hpp>
#include <rhi/ez_vulkan.h>

struct ScopedDrawLabel
{
    static float WHITE[4];
    static float RED[4];

    ScopedDrawLabel(const char* label_name, const float color[4]);
    ~ScopedDrawLabel();
};

struct FrameConstants
{
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
};

struct SceneTransform
{
    glm::mat4 transform;
    glm::mat4 pad0;
    glm::mat4 pad1;
    glm::mat4 pad2;
};

struct RenderView
{
    float zn;
    float zf;
    float ev100;
    float exposure;
    glm::vec3 position;
    glm::vec3 view_direction;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};