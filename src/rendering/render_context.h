#pragma once

#include "render_constants.h"
#include "render_resources.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Window;

class RenderContext
{
public:
    RenderContext();

    ~RenderContext();

    void collect_viewport_info(Window* window);

    enum class CreateStatus
    {
        Keep,
        Recreated,
    };

    UniformBuffer* find_ub(const std::string& name);
    UniformBuffer* create_ub(const std::string& name, uint32_t size, CreateStatus& status);

    TextureRef* find_texture_ref(const std::string& name);
    TextureRef* create_texture_ref(const std::string& name, const EzTextureDesc& desc, CreateStatus& status);

protected:
    void build_builtin_resources();

    void clear();

public:
    glm::vec4 viewport_size;

    VertexBuffer* sphere_vertex_buffer = nullptr;
    IndexBuffer* sphere_index_buffer = nullptr;
    float sphere_overfit = 0.0;

    VertexBuffer* cone_vertex_buffer = nullptr;
    IndexBuffer* cone_index_buffer = nullptr;
    float cone_overfit = 0.0;

    VertexBuffer* box_vertex_buffer = nullptr;
    IndexBuffer* box_index_buffer = nullptr;

private:
    std::unordered_map<std::string, UniformBuffer*> _ub_cache;
    std::unordered_map<std::string, TextureRef*> _t_ref_cache;
};