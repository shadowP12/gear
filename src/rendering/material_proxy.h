#pragma once

#include "render_resources.h"
#include "material_constants.h"
#include <rhi/ez_vulkan.h>
#include <glm/glm.hpp>
#include <memory>

struct MaterialParams
{
    glm::vec4 base_color;
};

class MaterialProxy
{
public:
    void make_dirty();
    void clear_dirty();
    void compilation_environment(std::vector<std::string>& macros);

public:
    bool dirty = false;
    MaterialParams params;
    EzTexture base_color_texture = VK_NULL_HANDLE;
    MaterialAlphaMode alpha_mode;
    std::shared_ptr<UniformBuffer> material_ub;
};

class MaterialProxyPool
{
public:
    MaterialProxyPool();
    ~MaterialProxyPool();

    int register_proxy();
    void unregister_proxy(int material_id);

    MaterialProxy* get_proxy(int material_id);
    void update_dirty_proxys();

private:
    std::vector<MaterialProxy> _proxys;
};