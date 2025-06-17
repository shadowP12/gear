#pragma once

#include <rhi/ez_vulkan.h>

struct DebugLabel {
    static float WHITE[4];
    static float RED[4];

    DebugLabel(const char* label_name, const float color[4]);

    ~DebugLabel();
};