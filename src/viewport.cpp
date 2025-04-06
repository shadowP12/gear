#include "viewport.h"

Viewport::Viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    set_size(x, y, w, h);
}

void Viewport::set_size(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    _size = glm::uvec4(x, y, w, h);
}

glm::uvec4 Viewport::get_size()
{
    return _size;
}

float Viewport::get_aspect()
{
    return (float)_size.z / (float)_size.w;
}