#pragma once
#include "../draw_command.h"
#include <math/bounding_box.h>
#include <memory>
#include <rhi/ez_vulkan.h>

#define SHADOW_CASCADE_COUNT 3

class Program;
class Renderable;
class RenderContext;

struct PerShadowInfo {
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

    void convert_to_vsm(RenderContext* ctx);

private:
    ShadowDrawCommandList _draw_list;
    std::vector<PerShadowInfo> _shadow_infos;
    std::unique_ptr<Program> _vsm_convert_program;
    std::unique_ptr<Program> _vsm_blur_vertical_program;
    std::unique_ptr<Program> _vsm_blur_horizontal_program;
};