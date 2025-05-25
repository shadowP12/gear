#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "features.h"
#include "rhi/ez_vulkan.h"

struct ProgramParameter
{
    uint32_t view = 0;
    uint32_t size = 0;
    uint32_t offset = 0;
    EzBuffer buffer = VK_NULL_HANDLE;
    EzTexture texture = VK_NULL_HANDLE;
    EzSampler sampler = VK_NULL_HANDLE;
};

struct ProgramDesc
{
    std::string vs;
    std::string fs;
    std::string cs;
    std::vector<std::string> macros;
    std::vector<Feature> features;
};

class Program
{
public:
    Program(const ProgramDesc& desc);

    ~Program();

    bool has_feature(const std::vector<Feature>& features);

    void reload();

    void bind();

    void set_parameter(const std::string& name, const void* data);

    void set_parameter(const std::string& name, EzBuffer buffer);

    void set_parameter(const std::string& name, EzBuffer buffer, uint32_t size, uint32_t offset = 0);

    void set_parameter(const std::string& name, EzTexture texture, uint32_t view = 0);

    void set_parameter(const std::string& name, EzTexture texture, EzSampler sampler, uint32_t view = 0);

private:
    void init_parameters();

private:
    EzShader _vs = VK_NULL_HANDLE;
    EzShader _fs = VK_NULL_HANDLE;
    EzShader _cs = VK_NULL_HANDLE;

    uint8_t* _parameter_data = nullptr;
    EzBuffer _parameter_buffer = VK_NULL_HANDLE;
    SpvReflectDescriptorBinding* _parameter_buffer_binding = nullptr;
    std::unordered_map<std::string, uint32_t> _members_lookup;

    std::unordered_map<uint32_t, ProgramParameter> _parameters;
    std::unordered_map<uint32_t, SpvReflectDescriptorBinding*> _parameters_binding;
    std::unordered_map<std::string, uint32_t> _parameters_lookup;

    ProgramDesc _desc;
};

class ProgramPool
{
public:
    void add_program(Program* program);

    void remove_program(Program* program);

    void reload(const std::vector<Feature>& features);

private:
    std::vector<Program*> _programs;
};

extern ProgramPool* g_program_pool;