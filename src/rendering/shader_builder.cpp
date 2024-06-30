#include "shader_builder.h"
#include <rhi/rhi_shader_mgr.h>

void ShaderBuilder::set_source(const std::string& src)
{
    _src = src;
}

void ShaderBuilder::set_vertex_factory(VertexFactory* vertex_factory)
{
    vertex_factory->compilation_environment(_macros);
}

void ShaderBuilder::set_material_proxy(MaterialProxy* material_proxy)
{
    material_proxy->compilation_environment(_macros);
}

EzShader ShaderBuilder::build()
{
    return rhi_get_shader(_src, _macros);
}