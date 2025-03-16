#pragma once
#include <core/module.h>
#include <core/event.h>
#include <rhi/ez_vulkan.h>
#include <memory>

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

private:
    RenderContext* _ctx;
};