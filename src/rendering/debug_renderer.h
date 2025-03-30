#pragma once

#include <rhi/ez_vulkan.h>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include "program.h"

class Window;
class RenderContext;
class Program;

class DebugRenderer
{
public:
    DebugRenderer();

    ~DebugRenderer();

    void render(RenderContext* ctx);

    void add_cone(const glm::vec3& col, const glm::vec3& pos, const glm::vec3& dir, float angle, float h);

    void add_sphere(const glm::vec3& col, const glm::vec3& pos, float r);

private:
    std::unique_ptr<Program> _geometry_program;
    std::vector<glm::vec4> _cones;
    std::vector<glm::vec4> _spheres;
};

extern DebugRenderer* g_debug_renderer;