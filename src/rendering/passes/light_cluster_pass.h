#pragma once

#include <memory>

class Program;
class RenderContext;

class LightClusterPass
{
public:
    LightClusterPass();

    ~LightClusterPass();

    void exec(RenderContext* ctx);

    void debug(RenderContext* ctx);

private:
    std::unique_ptr<Program> _debug_program;
    std::unique_ptr<Program> _aabb_program;
    std::unique_ptr<Program> _clustering_program;
};