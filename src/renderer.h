#pragma once

#include <rhi/ez_vulkan.h>
#include <glm/glm.hpp>

class Camera;

struct FrameConstants
{
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
};

class Renderer
{
public:
    Renderer();

    ~Renderer();

    void render(EzSwapchain swapchain, float dt);

    void set_camera(Camera* camera);

private:
    void update_rendertarget();

    void update_frame_constants_buffer();

    uint32_t _width = 0;
    uint32_t _height = 0;
    uint64_t _frame_number = 0;

    Camera* _camera = nullptr;

    EzBuffer _frame_constants_buffer = VK_NULL_HANDLE;
    EzTexture _color_rt = VK_NULL_HANDLE;
    EzTexture _depth_rt = VK_NULL_HANDLE;
};