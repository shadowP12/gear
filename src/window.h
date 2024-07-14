#pragma once

#include "viewport.h"
#include <rhi/ez_vulkan.h>
#include <imgui.h>
#include <imgui_internal.h>

class GLFWwindow;

class Window : public Viewport
{
public:
    Window(uint32_t width, uint32_t height);

    virtual ~Window();

    uint32_t get_width() { return _width; }

    uint32_t get_height() { return _height; }

    void set_title(const char* title);

    bool should_close();

    void new_frame(float dt);

    EzSwapchain get_swapchain();

    ImGuiContext* get_imgui_ctx() { return _imgui_ctx; }

    EzTexture get_font_texture() { return _font_texture; }

    static void glfw_imgui_init();

    static void glfw_imgui_terminate();

    static void glfw_poll_events();

    static double glfw_get_time();

    static void window_size_callback(GLFWwindow* window, int w, int h);

    static void cursor_position_callback(GLFWwindow* window, double pos_x, double pos_y);

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

    static void mouse_scroll_callback(GLFWwindow* window, double offset_x, double offset_y);

protected:
    virtual void draw_ui() {}

private:
    void internal_window_size_callback(int w, int h);

    void internal_cursor_position_callback(double pos_x, double pos_y);

    void internal_mouse_button_callback(int button, int action, int mods);

    void internal_mouse_scroll_callback(double offset_x, double offset_y);

protected:
    int _window_id = 0;
    void* _window_ptr = nullptr;
    GLFWwindow* _glfw_window = nullptr;
    uint32_t _width = 0;
    uint32_t _height = 0;
    EzSwapchain _swapchain = VK_NULL_HANDLE;
    ImGuiContext* _imgui_ctx = nullptr;
    EzTexture _font_texture = VK_NULL_HANDLE;
};