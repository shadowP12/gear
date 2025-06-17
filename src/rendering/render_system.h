#pragma once
#include "features.h"
#include <core/event.h>
#include <core/module.h>
#include <memory>
#include <rhi/ez_vulkan.h>

class World;
class Window;
class RenderContext;

class RenderSystem : public Module<RenderSystem>
{
public:
    RenderSystem() = default;

    ~RenderSystem() = default;

    void setup();

    void finish();

    void render(Window* window);

public:
    Event<> predraw_event;
    FeatureConfig feature_config;

private:
    RenderContext* _ctx;
};