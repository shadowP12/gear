#include "main_window.h"
#include "world.h"

MainWindow::MainWindow(uint32_t width, uint32_t height)
    : Window(width, height)
{
}

MainWindow::~MainWindow()
{
}

void MainWindow::set_current_world(World* world)
{
    _world = world;
}

void MainWindow::draw_ui()
{
    if (!_world)
        return;

    
}