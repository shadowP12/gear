#pragma once

#include <glm/glm.hpp>

class Viewport
{
public:
    Viewport(int x, int y, int w, int h);

    glm::vec4 get_size();

    void set_size(int x, int y, int w, int h);

    float get_aspect();

private:
    int _width;
    int _hight;
    glm::vec4 _size;
};