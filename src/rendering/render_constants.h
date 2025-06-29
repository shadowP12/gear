#pragma once

#include <glm/glm.hpp>

struct FrameConstants {
    glm::uvec4 cluster_size;
    glm::vec4 view_position;
    glm::vec4 view_direction;
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
    glm::mat4 inv_view_proj_matrix;
    glm::vec2 z_near_far;
    float exposure;
    bool has_sun;
};

struct SceneInstanceData {
    glm::mat4 transform;
    glm::mat4 pad0;
    glm::mat4 pad1;
    glm::mat4 pad2;
};

enum class SamplerType {
    NearestClamp = 0,
    LinearClamp,
    Shadow,
    Count
};

enum DrawType {
    DRAW_OPAQUE = 0,
    DRAW_SHADOW,
    DRAW_MAXCOUNT
};