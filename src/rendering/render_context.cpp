#include "render_context.h"

RenderContext::RenderContext()
{}

RenderContext::~RenderContext()
{}

void RenderContext::update(float dt)
{
    delta_time = dt;
}