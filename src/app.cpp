#include "app.h"
#include "world.h"
#include "window.h"
#include "main_window.h"
#include "asset/level.h"
#include "asset/asset_manager.h"
#include "entity/entity.h"
#include "entity/components/c_camera.h"
#include "rendering/render_system.h"
#include "importer/gltf_importer.h"
#include "gameplay/camera_controller.h"
#include <core/path.h>
#include <rhi/ez_vulkan.h>
#include <rhi/rhi_shader_mgr.h>
#include <core/memory.h>

void Application::setup(const ApplicationSetting& setting)
{
    Path::register_protocol("content", std::string(PROJECT_DIR) + "/content/");
    Path::register_protocol("asset", std::string(PROJECT_DIR) + "/content/gear_asset/");
    Path::register_protocol("shader", std::string(PROJECT_DIR) + "/content/shader/");

    ez_init();
    rhi_shader_mgr_init();
    ez_create_query_pool(16, VK_QUERY_TYPE_TIMESTAMP, _timestamp_query_pool);

    RenderSystem::get()->setup();
    AssetManager::get()->setup();

    Window::glfw_imgui_init();
    _window = new MainWindow(setting.window_width, setting.window_height);
    _window->set_current_app(this);

    _world = new World();
    _world->set_viewport(_window);

    _camera_controller = new CameraController();
    auto& camera_entities = _world->get_camera_entities();
    if (!camera_entities.empty())
    {
        _camera_controller->set_camera(camera_entities[0]);
    }
}

void Application::exit()
{
    SAFE_DELETE(_camera_controller);
    SAFE_DELETE(_world);

    AssetManager::get()->finish();
    RenderSystem::get()->finish();

    ez_destroy_query_pool(_timestamp_query_pool);

    SAFE_DELETE(_window);
    Window::glfw_imgui_terminate();

    rhi_shader_mgr_terminate();
    ez_flush();
    ez_terminate();
}

bool Application::should_close()
{
    return _window->should_close();
}

void Application::run()
{
    Window::glfw_poll_events();

    double current_time = Window::glfw_get_time();
    float dt = _time > 0.0 ? (float)(current_time - _time) : (float)(1.0f / 60.0f);
    _time = current_time;

    tick(dt);
}

void Application::tick(float dt)
{
    _world->tick(dt);

    _window->new_frame(dt);

    ez_reset_query_pool(_timestamp_query_pool, 0, 16);
    ez_write_timestamp(_timestamp_query_pool, 0);

    RenderSystem::get()->render(_window);

    ez_write_timestamp(_timestamp_query_pool, 1);
    ez_submit();

    uint64_t timestamp_results[2] = {};
    ez_get_query_pool_results(_timestamp_query_pool, 0, 2, sizeof(timestamp_results), timestamp_results, sizeof(timestamp_results[0]), VK_QUERY_RESULT_64_BIT);

    double frame_gpu_begin = double(timestamp_results[0]) * ez_get_timestamp_period() * 1e-6;
    double frame_gpu_end = double(timestamp_results[1]) * ez_get_timestamp_period() * 1e-6;
    _frame_gpu_avg = _frame_gpu_avg * 0.95 + (frame_gpu_end - frame_gpu_begin) * 0.05;

    char title[256];
    snprintf(title, sizeof(title), "gpu: %.2f ms", _frame_gpu_avg);
    _window->set_title(title);
}