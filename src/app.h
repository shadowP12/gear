#pragma once

#include <rhi/ez_vulkan.h>

struct ApplicationSetting
{
    int window_width = 1024;
    int window_height = 768;
    int window_pos_x = 0;
    int window_pos_y = 0;
};

class Application
{
public:
    Application() = default;
    ~Application() = default;

    void setup(const ApplicationSetting& setting);
    void exit();
    bool should_close();
    void run();

protected:
    void tick(float dt);

protected:
    struct GLFWwindow* _window_ptr = nullptr;
    class Camera* _main_camera = nullptr;
    class CameraController* _camera_controller = nullptr;
    class Renderer* _renderer = nullptr;

    EzSwapchain _swapchain = VK_NULL_HANDLE;
    EzQueryPool _timestamp_query_pool = VK_NULL_HANDLE;

    double _time = 0.0;
    double _frame_gpu_avg = 0.0;
};