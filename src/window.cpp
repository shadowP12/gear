#include "window.h"
#include <core/path.h>
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

    _imgui_ctx = ImGui::CreateContext();
    ImGuiIO& io = _imgui_ctx->IO;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendPlatformName = "glfw";
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
    io.Fonts->AddFontFromFileTTF(Path::fix_path("content://fonts/Roboto-Medium.ttf").c_str(), 16.0f);
    io.Fonts->Build();
}

Window::~Window()
{
    ImGui::DestroyContext(_imgui_ctx);

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

void Window::glfw_imgui_init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void Window::glfw_imgui_terminate()
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

void Window::new_frame(float dt)
{
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(_glfw_window, &w, &h);
    glfwGetFramebufferSize(_glfw_window, &display_w, &display_h);
    if (w <= 0 || h <= 0)
        return;

    ImGui::SetCurrentContext(_imgui_ctx);

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = dt;
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
    {
        io.MouseDown[i] = glfwGetMouseButton(_glfw_window, i) != 0;
    }

    double mouse_x, mouse_y;
    glfwGetCursorPos(_glfw_window, &mouse_x, &mouse_y);
    io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);

    ImGui::NewFrame();
    draw_ui();
    ImGui::Render();
}