#include "debug_utils.h"

float DebugLabel::WHITE[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float DebugLabel::RED[4] = {1.0f, 0.0f, 0.0f, 1.0f};

DebugLabel::DebugLabel(const char* label_name, const float color[4])
{
    ez_begin_debug_label(label_name, color);
}

DebugLabel::~DebugLabel()
{
    ez_end_debug_label();
}