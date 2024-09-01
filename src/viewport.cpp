#include "viewport.h"

Viewport::Viewport(int x, int y, int w, int h)
{
    set_size(x, y, w, h);
}

void Viewport::set_size(int x, int y, int w, int h)
{
    _size = glm::vec4(x, y, w, h);
    _width = w - x;
    _hight = h - y;
}

glm::vec4 Viewport::get_size()
{
    return _size;
}

float Viewport::get_aspect()
{
    return (float)_width / (float)_hight;
}