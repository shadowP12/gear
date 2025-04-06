#pragma once

#include <glm/glm.hpp>

class Viewport
{
public:
    Viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

    glm::uvec4 get_size();

    void set_size(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

    float get_aspect();

private:
    glm::uvec4 _size;
};