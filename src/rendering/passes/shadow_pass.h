#pragma once
#include "../draw_command.h"
#include <math/bounding_box.h>
#include <rhi/ez_vulkan.h>

#define SHADOW_CASCADE_COUNT 2

class Program;
class Renderable;
class RenderContext;

struct PerShadowInfo
{
    glm::mat4 light_matrixs[6];
    glm::vec4 cascade_splits;
};

class ShadowDrawCommandList : public DrawCommandList
{
public:
    void draw(RenderContext* ctx) override;

    std::string u_pass_name;
};

class ShadowPass
{
public:
    ShadowPass();

    ~ShadowPass();

    void setup(RenderContext* ctx);

    void process(RenderContext* ctx, Renderable* renderable);

    void exec(RenderContext* ctx);

private:
    ShadowDrawCommandList _draw_list;
    std::vector<PerShadowInfo> _shadow_infos;
};