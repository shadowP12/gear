#include "shader_builder.h"
#include <rhi/rhi_shader_mgr.h>

void ShaderBuilder::set_source(const std::string& src)
{
    _src = src;
}

void ShaderBuilder::set_vertex_factory(int vertex_factory_id)
{
}

void ShaderBuilder::set_material_proxy(MaterialProxy* material_proxy)
{
    material_proxy->modify_compilation_environment(_macros);
}

EzShader ShaderBuilder::build()
{
    return rhi_get_shader(_src, _macros);
}