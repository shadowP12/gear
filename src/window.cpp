#include "window.h"
#include <input/input_events.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <map>

static int g_window_id = 0;
static std::map<GLFWwindow*, Window*> glfw_window_table;

Window::Window(uint32_t width, uint32_t height)
    : Viewport(0, 0, width, height)
{
    _window_id = g_window_id++;
    _glfw_window = glfwCreateWindow(width, height, "Gear", nullptr, nullptr);
    _width = width;
    _height = height;
    _window_ptr = glfwGetWin32Window(_glfw_window);
    glfwSetFramebufferSizeCallback(_glfw_window, window_size_callback);
    glfwSetCursorPosCallback(_glfw_window, cursor_position_callback);
    glfwSetMouseButtonCallback(_glfw_window, mouse_button_callback);
    glfwSetScrollCallback(_glfw_window, mouse_scroll_callback);
    glfw_window_table[_glfw_window] = this;

    ez_create_swapchain(_window_ptr, _swapchain);
}

Window::~Window()
{
    ez_destroy_swapchain(_swapchain);
    glfwDestroyWindow(_glfw_window);
}

bool Window::should_close()
{
    return glfwWindowShouldClose(_glfw_window);
};

void Window::set_title(const char* title)
{
    glfwSetWindowTitle(_glfw_window, title);
}

void Window::glfw_init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void Window::glfw_terminate()
{
    glfwTerminate();
}

void Window::glfw_poll_events()
{
    glfwPollEvents();
}

double Window::glfw_get_time()
{
    return glfwGetTime();
}

void Window::window_size_callback(GLFWwindow* window, int w, int h)
{
    glfw_window_table[window]->internal_window_size_callback(w, h);
}

void Window::cursor_position_callback(GLFWwindow* window, double pos_x, double pos_y)
{
    glfw_window_table[window]->internal_cursor_position_callback(pos_x, pos_y);
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    glfw_window_table[window]->internal_mouse_button_callback(button, action, mods);
}

void Window::mouse_scroll_callback(GLFWwindow* window, double offset_x, double offset_y)
{
    glfw_window_table[window]->internal_mouse_scroll_callback(offset_x, offset_y);
}

void Window::internal_window_size_callback(int w, int h)
{
    _width = w;
    _height = h;
    set_size(0, 0, _width, _height);
}

void Window::internal_cursor_position_callback(double pos_x, double pos_y)
{
    MouseEvent mouse_event;
    mouse_event.window_id = _window_id;
    mouse_event.type = MouseEvent::Type::MOVE;
    mouse_event.x = (float)pos_x;
    mouse_event.y = (float)pos_y;
    Input::get_mouse_event().broadcast(mouse_event);
}

void Window::internal_mouse_button_callback(int button, int action, int mods)
{
    double pos_x, pos_y;
    glfwGetCursorPos(_glfw_window, &pos_x, &pos_y);

    if (action == 0)
    {
        MouseEvent mouse_event;
        mouse_event.window_id = _window_id;
        mouse_event.type = MouseEvent::Type::UP;
        mouse_event.x = (float)pos_x;
        mouse_event.y = (float)pos_y;
        mouse_event.button = button;
        Input::get_mouse_event().broadcast(mouse_event);
    }
    else if (action == 1)
    {
        MouseEvent mouse_event;
        mouse_event.window_id = _window_id;
        mouse_event.type = MouseEvent::Type::DOWN;
        mouse_event.x = (float)pos_x;
        mouse_event.y = (float)pos_y;
        mouse_event.button = button;
        Input::get_mouse_event().broadcast(mouse_event);
    }
}

void Window::internal_mouse_scroll_callback(double offset_x, double offset_y)
{
    MouseEvent mouse_event;
    mouse_event.window_id = _window_id;
    mouse_event.type = MouseEvent::Type::WHEEL;
    mouse_event.offset_x = (float)offset_x;
    mouse_event.offset_y = (float)offset_y;
    Input::get_mouse_event().broadcast(mouse_event);
}

EzSwapchain Window::get_swapchain()
{
    EzSwapchainStatus swapchain_status = ez_update_swapchain(_swapchain);

    if (swapchain_status == EzSwapchainStatus::NotReady)
        return VK_NULL_HANDLE;

    return _swapchain;
}