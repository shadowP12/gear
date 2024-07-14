#pragma once

#include "window.h"

class Application;

class MainWindow : public Window
{
public:
    MainWindow(uint32_t width, uint32_t height);

    virtual ~MainWindow();

    void set_current_app(Application* app);

protected:
    virtual void draw_ui();

private:
    Application* _app = nullptr;
};