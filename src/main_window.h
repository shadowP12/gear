#pragma once

#include "window.h"

class World;

class MainWindow : public Window
{
public:
    MainWindow(uint32_t width, uint32_t height);

    virtual ~MainWindow();

    void set_current_world(World* world);

protected:
    virtual void draw_ui();

private:
    World* _world = nullptr;
};