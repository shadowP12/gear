#include "program.h"
#include <rhi/rhi_shader_mgr.h>
#include <math/math_define.h>

static void reset_parameter(ProgramParameter& parameter)
{
    parameter = ProgramParameter();
}

Program::Program(const std::string& vs, const std::string& fs)
{
    std::vector<std::string> macros;
    _vs = rhi_get_shader(vs, macros);
    _fs = rhi_get_shader(fs, macros);
    init_parameters();
}

Program::Program(const std::string& vs, const std::string& fs, const std::vector<std::string>& macros)
{
    _vs = rhi_get_shader(vs, macros);
    _fs = rhi_get_shader(fs, macros);
    init_parameters();
}

Program::Program(const std::string& cs)
{
    std::vector<std::string> macros;
    _cs = rhi_get_shader(cs, macros);
    init_parameters();
}

Program::Program(const std::string& cs, const std::vector<std::string>& macros)
{
    _cs = rhi_get_shader(cs, macros);
    init_parameters();
}

Program::~Program()
{
    if (_parameter_buffer)
    {
        ez_unmap_memory(_parameter_buffer);
        _parameter_data = nullptr;
        ez_destroy_buffer(_parameter_buffer);
    }
}

void Program::init_parameters()
{
    auto process_shader_func = [&](EzShader shader)
    {
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

        if (descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
            descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
        {
            ez_bind_buffer(parameter_binding->binding, parameter.buffer, parameter.size, parameter.offset);
        }
        else if( descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
                 descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        {
            if ( parameter_binding->image.arrayed > 0)
            {
                for (int i = 0; i < parameter.array_count; ++i)
                {
                    ez_bind_texture_array(parameter_binding->binding, parameter.textures[i], parameter.views[i], i);
                }
            }
            else
            {
                ez_bind_texture(parameter_binding->binding, parameter.textures[0], parameter.views[0]);
            }
        }
        else if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
        {
            if ( parameter_binding->image.arrayed > 0)
            {
                for (int i = 0; i < parameter.array_count; ++i)
                {
                    ez_bind_texture_array(parameter_binding->binding, parameter.textures[i], parameter.views[i], i);
                    ez_bind_sampler_array(parameter_binding->binding, parameter.samplers[i], i);
                }
            }
            else
            {
                ez_bind_texture(parameter_binding->binding, parameter.textures[0], parameter.views[0]);
                ez_bind_sampler(parameter_binding->binding, parameter.samplers[0]);
            }
        }

        // Reset parameter
        reset_parameter(parameter);
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
    _parameters[binding].textures[0] = texture;
    _parameters[binding].views[0] = view;
}

void Program::set_parameter(const std::string& name, EzTexture texture, EzSampler sampler, uint32_t view)
{
    uint32_t binding = _parameters_lookup[name];
    _parameters[binding].textures[0] = texture;
    _parameters[binding].samplers[0] = sampler;
    _parameters[binding].views[0] = view;
}

void Program::set_parameter_array(const std::string& name, EzTexture texture, uint32_t view, uint32_t array)
{
    uint32_t binding = _parameters_lookup[name];
    _parameters[binding].textures[array] = texture;
    _parameters[binding].views[array] = view;
    _parameters[binding].array_count = glm::max(array, _parameters[binding].array_count);
}

void Program::set_parameter_array(const std::string& name, EzTexture texture, EzSampler sampler, uint32_t view, uint32_t array)
{
    uint32_t binding = _parameters_lookup[name];
    _parameters[binding].textures[array] = texture;
    _parameters[binding].samplers[array] = sampler;
    _parameters[binding].views[array] = view;
    _parameters[binding].array_count = glm::max(array, _parameters[binding].array_count);
}