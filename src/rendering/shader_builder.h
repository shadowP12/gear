#pragma once

#include "vertex_factory.h"
#include "material_proxy.h"
#include <rhi/ez_vulkan.h>
#include <vector>

class ShaderBuilder
{
public:
    ShaderBuilder() = default;
    ~ShaderBuilder() = default;

    void set_source(const std::string& src);

    void set_vertex_factory(int vertex_factory_id);

    void set_material_proxy(MaterialProxy* material_proxy);

    EzShader build();

private:
    std::string _src;
    std::vector<std::string> _macros;
};