#pragma once

#include <rhi/ez_vulkan.h>
#include <memory>
#include <string>

struct ApplicationSetting
{
    int window_width = 1024;
    int window_height = 768;
    int window_pos_x = 0;
    int window_pos_y = 0;
};

class World;
class Window;
class MainWindow;
class CameraController;

class Application
{
public:
    Application() = default;
    ~Application() = default;

    void setup(const ApplicationSetting& setting);
    void exit();
    bool should_close();
    void run();

    World* get_world() { return _world; }

protected:
    void tick(float dt);

protected:
    World* _world = nullptr;
    MainWindow* _window = nullptr;
    CameraController* _camera_controller = nullptr;

    EzQueryPool _timestamp_query_pool = VK_NULL_HANDLE;

    double _time = 0.0;
    double _frame_gpu_avg = 0.0;
};