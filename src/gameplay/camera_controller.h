#pragma once

#include <set>
#include <core/event.h>
#include <input/input_events.h>
#include <glm/glm.hpp>

class Entity;
class CTransform;
class CameraController
{
public:
    CameraController();

    ~CameraController();

    void tick(float dt);

    void set_camera(Entity* camera);

    Entity* get_camera() { return _camera; };

private:
    void begin(float x, float y)
    {
        _grabbing = true;
        _start_point = glm::vec2(x, y);
    }

    void end()
    {
        _grabbing = false;
    }

    void on_mouse_event_received(MouseEvent mouse_event);

    void on_keyboard_event_received(KeyboardEvent keyboard_event);

private:
    Entity* _camera = nullptr;
    bool _grabbing = false;
    glm::vec2 _start_point;
    std::set<KeyCode> _pressed_keys;
    EventHandle _mouse_event_handle;
    EventHandle _keyboard_event_handle;
};