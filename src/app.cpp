#include "app.h"
#include "camera.h"
#include "camera_controller.h"
#include "renderer.h"

#include <core/path.h>
#include <input/input_events.h>
#include <rhi/ez_vulkan.h>
#include <rhi/rhi_shader_mgr.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

static void cursor_position_callback(GLFWwindow* window, double pos_x, double pos_y)
{
    MouseEvent mouse_event;
    mouse_event.type = MouseEvent::Type::MOVE;
    mouse_event.x = (float)pos_x;
    mouse_event.y = (float)pos_y;
    Input::mouse_event.broadcast(mouse_event);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double pos_x, pos_y;
    glfwGetCursorPos(window, &pos_x, &pos_y);

    if (action == 0)
    {
        MouseEvent mouse_event;
        mouse_event.type = MouseEvent::Type::UP;
        mouse_event.x = (float)pos_x;
        mouse_event.y = (float)pos_y;
        Input::mouse_event.broadcast(mouse_event);
    }
    else if (action == 1)
    {
        MouseEvent mouse_event;
        mouse_event.type = MouseEvent::Type::DOWN;
        mouse_event.x = (float)pos_x;
        mouse_event.y = (float)pos_y;
        Input::mouse_event.broadcast(mouse_event);
    }
}

static void mouse_scroll_callback(GLFWwindow* window, double offset_x, double offset_y)
{
    MouseEvent mouse_event;
    mouse_event.type = MouseEvent::Type::WHEEL;
    mouse_event.x = (float)offset_x;
    mouse_event.y = (float)offset_y;
    Input::mouse_event.broadcast(mouse_event);
}

void Application::setup(const ApplicationSetting& setting)
{
    Path::register_protocol("content", std::string(PROJECT_DIR) + "/content/");
    Path::register_protocol("shader", std::string(PROJECT_DIR) + "/content/shader/");

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window_ptr = glfwCreateWindow(setting.window_width, setting.window_height, "app", nullptr, nullptr);
    glfwSetWindowPos(_window_ptr, setting.window_pos_x, setting.window_pos_y);
    glfwSetCursorPosCallback(_window_ptr, cursor_position_callback);
    glfwSetMouseButtonCallback(_window_ptr, mouse_button_callback);
    glfwSetScrollCallback(_window_ptr, mouse_scroll_callback);

    ez_init();
    rhi_shader_mgr_init();

    ez_create_swapchain(glfwGetWin32Window(_window_ptr), _swapchain);
    ez_create_query_pool(16, VK_QUERY_TYPE_TIMESTAMP, _timestamp_query_pool);

    _main_camera = new Camera();
    _main_camera->set_aspect((float)setting.window_width / (float)setting.window_height);
    _main_camera->set_translation(glm::vec3(0.0f, -2.0f, 18.0f));
    _main_camera->set_euler(glm::vec3(0.0f, 0.0f, 0.0f));
    _camera_controller = new CameraController();
    _camera_controller->set_camera(_main_camera);

    _renderer = new Renderer();
    _renderer->set_camera(_main_camera);
}

void Application::exit()
{
    delete _renderer;
    delete _main_camera;
    delete _camera_controller;

    ez_destroy_swapchain(_swapchain);
    ez_destroy_query_pool(_timestamp_query_pool);

    glfwDestroyWindow(_window_ptr);
    glfwTerminate();

    rhi_shader_mgr_terminate();
    ez_flush();
    ez_terminate();
}

bool Application::should_close()
{
    return glfwWindowShouldClose(_window_ptr);
}

void Application::run()
{
    glfwPollEvents();

    double current_time = glfwGetTime();
    float dt = _time > 0.0 ? (float)(current_time - _time) : (float)(1.0f / 60.0f);
    _time = current_time;
    tick(dt);
}

void Application::tick(float dt)
{
    EzSwapchainStatus swapchain_status = ez_update_swapchain(_swapchain);

    if (swapchain_status == EzSwapchainStatus::NotReady)
        return;

    if (swapchain_status == EzSwapchainStatus::Resized) {
        _main_camera->set_aspect(_swapchain->width / _swapchain->height);
    }

    ez_acquire_next_image(_swapchain);

    ez_reset_query_pool(_timestamp_query_pool, 0, 16);
    ez_write_timestamp(_timestamp_query_pool, 0);

    _renderer->render(_swapchain, dt);

    VkImageMemoryBarrier2 present_barrier[] = { ez_image_barrier(_swapchain, EZ_RESOURCE_STATE_PRESENT) };
    ez_pipeline_barrier(0, 0, nullptr, 1, present_barrier);

    ez_present(_swapchain);

    ez_write_timestamp(_timestamp_query_pool, 1);
    ez_submit();

    uint64_t timestamp_results[2] = {};
    ez_get_query_pool_results(_timestamp_query_pool, 0, 2, sizeof(timestamp_results), timestamp_results, sizeof(timestamp_results[0]), VK_QUERY_RESULT_64_BIT);

    double frame_gpu_begin = double(timestamp_results[0]) * ez_get_timestamp_period() * 1e-6;
    double frame_gpu_end = double(timestamp_results[1]) * ez_get_timestamp_period() * 1e-6;
    _frame_gpu_avg = _frame_gpu_avg * 0.95 + (frame_gpu_end - frame_gpu_begin) * 0.05;

    char title[256];
    snprintf(title, sizeof(title), "gpu: %.2f ms", _frame_gpu_avg);
    glfwSetWindowTitle(_window_ptr, title);
}