#include "camera_controller.h"
#include "entity/entity.h"
#include "entity/components/c_camera.h"
#include <input/input_events.h>

CameraController::CameraController()
{
    _mouse_event_handle = Input::get_mouse_event().bind(EVENT_CB(CameraController::on_mouse_event_received));
}

CameraController::~CameraController()
{
    Input::get_mouse_event().unbind(_mouse_event_handle);
}

void CameraController::set_camera(Entity* camera)
{
    if (!camera->has_component<CCamera>())
        return;

    _camera = camera;
}

void CameraController::on_mouse_event_received(MouseEvent mouse_event)
{
    if (!_camera)
        return;

    if (mouse_event.type == MouseEvent::Type::MOVE)
    {
        if (_grabbing)
        {
            glm::vec2 mouse_position = glm::vec2(mouse_event.x, mouse_event.y);
            glm::vec2 offset = _start_point - mouse_position;
            _start_point = mouse_position;


            const float turn_rate = 0.001f;
            glm::vec3 euler = _camera->get_euler();
            euler.x += offset.y * turn_rate;
            euler.y += offset.x * turn_rate;
            _camera->set_euler(euler);
        }
    }
    else if (mouse_event.type == MouseEvent::Type::UP)
    {
        if (mouse_event.button == 1)
        {
            end();
        }
    }
    else if (mouse_event.type == MouseEvent::Type::DOWN)
    {
        if (mouse_event.button == 1)
        {
            begin(mouse_event.x, mouse_event.y);
        }
    }
    else if (mouse_event.type == MouseEvent::Type::WHEEL)
    {
        glm::vec3 camera_pos = _camera->get_translation();
        glm::vec3 camera_front = _camera->get_front_vector();

        const float speed = 0.4f;
        camera_pos += camera_front * mouse_event.offset_y * speed;
        _camera->set_translation(camera_pos);
    }
}