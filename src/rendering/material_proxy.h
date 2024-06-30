#pragma once

#include "render_resources.h"
#include "material_constants.h"
#include <rhi/ez_vulkan.h>
#include <glm/glm.hpp>
#include <memory>

class MaterialProxy
{
public:
    MaterialProxy();

    ~MaterialProxy();

    void make_dirty();

    void clear_dirty();

    void compilation_environment(std::vector<std::string>& macros);

    void bind();

public:
    bool dirty = false;
    int material_id = 0;
    MaterialParams params;
    EzTexture base_color_texture = VK_NULL_HANDLE;
    MaterialAlphaMode alpha_mode;
    UniformBuffer* material_ub = nullptr;
};