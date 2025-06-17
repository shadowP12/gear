#include "program.h"
#include <math/math_define.h>
#include <rhi/rhi_shader_mgr.h>

ProgramPool* g_program_pool = nullptr;

Program::Program(const ProgramDesc& desc)
{
    _desc = desc;
    reload();
    init_parameters();
    g_program_pool->add_program(this);
}

Program::~Program()
{
    g_program_pool->remove_program(this);
    if (_parameter_buffer)
    {
        ez_unmap_memory(_parameter_buffer);
        _parameter_data = nullptr;
        ez_destroy_buffer(_parameter_buffer);
    }
}

bool Program::has_feature(const std::vector<Feature>& features)
{
    for (auto feature : _desc.features)
    {
        if (std::find(features.begin(), features.end(), feature) != features.end())
        {
            return true;
        }
    }
    return false;
}

void Program::reload()
{
    std::vector<std::string> total_macros = _desc.macros;
    g_feature_config.get_macros(_desc.features, total_macros);

    if (!_desc.vs.empty())
    {
        _vs = rhi_get_shader(_desc.vs, total_macros);
    }
    if (!_desc.fs.empty())
    {
        _fs = rhi_get_shader(_desc.fs, total_macros);
    }
    if (!_desc.cs.empty())
    {
        _cs = rhi_get_shader(_desc.cs, total_macros);
    }
    init_parameters();
}

void Program::init_parameters()
{
    auto process_shader_func = [&](EzShader shader) {
        if (!shader)
        {
            return;
        }

        uint32_t binding_count = 0;
        spvReflectEnumerateDescriptorBindings(&shader->reflect, &binding_count, nullptr);

        std::vector<SpvReflectDescriptorBinding*> bindings(binding_count);
        spvReflectEnumerateDescriptorBindings(&shader->reflect, &binding_count, bindings.data());

        for (int i = 0; i < binding_count; ++i)
        {
            SpvReflectDescriptorBinding* binding = bindings[i];

            if (_parameters_binding.find(binding->binding) != _parameters_binding.end())
            {
                continue;
            }

            _parameters[binding->binding] = ProgramParameter();
            _parameters_binding[binding->binding] = binding;
            _parameters_lookup[binding->name] = binding->binding;

            if (!_parameter_buffer_binding && strcmp(binding->name, "u_params") == 0)
            {
                _parameter_buffer_binding = binding;

                for (int j = 0; j < _parameter_buffer_binding->block.member_count; ++j)
                {
                    _members_lookup[_parameter_buffer_binding->block.members[j].name] = j;
                }

                EzBufferDesc buffer_desc{};
                buffer_desc.size = _parameter_buffer_binding->block.size;
                buffer_desc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                buffer_desc.memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
                ez_create_buffer(buffer_desc, _parameter_buffer);

                ez_map_memory(_parameter_buffer, (void**)&_parameter_data);
            }
        }
    };

    process_shader_func(_vs);
    process_shader_func(_fs);
    process_shader_func(_cs);
}

void Program::bind()
{
    if (_vs)
    {
        ez_set_vertex_shader(_vs);
    }

    if (_fs)
    {
        ez_set_fragment_shader(_fs);
    }

    if (_cs)
    {
        ez_set_compute_shader(_cs);
    }

    if (_parameter_buffer_binding)
    {
        ez_bind_buffer(_parameter_buffer_binding->binding, _parameter_buffer);
    }

    for (auto& iter : _parameters)
    {
        auto parameter_binding = _parameters_binding[iter.first];
        auto descriptor_type = parameter_binding->descriptor_type;
        ProgramParameter& parameter = iter.second;

        if (_parameter_buffer_binding && _parameter_buffer_binding->binding == parameter_binding->binding)
        {
            continue;
        }

        if (descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
        {
            ez_bind_buffer(parameter_binding->binding, parameter.buffer, parameter.size, parameter.offset);
        }
        else if (descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE || descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        {
            ez_bind_texture(parameter_binding->binding, parameter.texture, parameter.view);
        }
        else if (descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
        {
            ez_bind_texture(parameter_binding->binding, parameter.texture, parameter.view);
            ez_bind_sampler(parameter_binding->binding, parameter.sampler);
        }
    }
}

void Program::set_parameter(const std::string& name, const void* data)
{
    auto member = _parameter_buffer_binding->block.members[_members_lookup[name]];
    memcpy(_parameter_data + member.offset, data, member.size);
}

void Program::set_parameter(const std::string& name, EzBuffer buffer)
{
    uint32_t binding = _parameters_lookup[name];
    _parameters[binding].buffer = buffer;
    _parameters[binding].size = buffer->size;
    _parameters[binding].offset = 0;
}

void Program::set_parameter(const std::string& name, EzBuffer buffer, uint32_t size, uint32_t offset)
{
    uint32_t binding = _parameters_lookup[name];
    _parameters[binding].buffer = buffer;
    _parameters[binding].size = size;
    _parameters[binding].offset = offset;
}

void Program::set_parameter(const std::string& name, EzTexture texture, uint32_t view)
{
    uint32_t binding = _parameters_lookup[name];
    _parameters[binding].texture = texture;
    _parameters[binding].view = view;
}

void Program::set_parameter(const std::string& name, EzTexture texture, EzSampler sampler, uint32_t view)
{
    uint32_t binding = _parameters_lookup[name];
    _parameters[binding].texture = texture;
    _parameters[binding].sampler = sampler;
    _parameters[binding].view = view;
}

void ProgramPool::add_program(Program* program)
{
    _programs.push_back(program);
}

void ProgramPool::remove_program(Program* program)
{
    auto iter = std::find(_programs.begin(), _programs.end(), program);
    if (iter != _programs.end())
    {
        _programs.erase(iter);
    }
}

void ProgramPool::reload(const std::vector<Feature>& features)
{
    for (auto program : _programs)
    {
        if (program->has_feature(features))
        {
            program->reload();
        }
    }
}