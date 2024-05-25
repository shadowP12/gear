#pragma once

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

    void set_camera(Entity* camera);

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

    CTransform* _camera_transform = nullptr;
    bool _grabbing = false;
    glm::vec2 _start_point;
    EventHandle _mouse_event_handle;
};