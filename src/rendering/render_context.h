#pragma once

#include "render_constants.h"

class RenderContext
{
public:
    RenderContext();
    ~RenderContext();

    void update(float dt);

public:
    float delta_time;
};