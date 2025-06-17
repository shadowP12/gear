#pragma once

#include <memory>
#include <rhi/ez_vulkan.h>

class Program;
class RenderContext;

class PostProcessPass
{
public:
    PostProcessPass();

    ~PostProcessPass();

    void exec(RenderContext* ctx);

    EzTexture get_final_rt(RenderContext* ctx);

private:
    bool _swap_output;
    std::unique_ptr<Program> _tonemapping_program;
};