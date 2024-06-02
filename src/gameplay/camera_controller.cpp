#include "camera_controller.h"
#include "entity/entity.h"
#include "entity/components/c_camera.h"
#include "entity/components/c_transform.h"
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

    _camera_transform = camera->get_component<CTransform>();
}

void CameraController::on_mouse_event_received(MouseEvent mouse_event)
{
    if (mouse_event.type == MouseEvent::Type::MOVE)
    {
        if (_grabbing)
        {
            glm::vec2 mouse_position = glm::vec2(mouse_event.x, mouse_event.y);
            glm::vec2 offset = _start_point - mouse_position;
            _start_point = mouse_position;


            const float turn_rate = 0.001f;
            glm::vec3 euler = _camera_transform->get_euler();
            euler.x += offset.y * turn_rate;
            euler.y += offset.x * turn_rate;
            _camera_transform->set_euler(euler);
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
        glm::vec3 camera_pos = _camera_transform->get_position();
        glm::vec3 camera_front = _camera_transform->get_front_vector();

        const float speed = 0.2f;
        camera_pos -= camera_front * mouse_event.offset_y * speed;
        _camera_transform->set_position(camera_pos);
    }
}