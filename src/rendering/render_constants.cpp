#include "render_constants.h"

float ScopedDrawLabel::WHITE[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float ScopedDrawLabel::RED[4] = {1.0f, 0.0f, 0.0f, 1.0f};

ScopedDrawLabel::ScopedDrawLabel(const char* label_name, const float color[4])
{
    ez_begin_debug_label(label_name, color);
}

ScopedDrawLabel::~ScopedDrawLabel()
{
    ez_end_debug_label();
}