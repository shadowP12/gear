#include "camera_controller.h"
#include "entity/entity.h"
#include "entity/components/c_camera.h"
#include <core/log.h>
#include <input/input_events.h>

CameraController::CameraController()
{
    _mouse_event_handle = Input::get_mouse_event().bind(EVENT_CB(CameraController::on_mouse_event_received));
    _keyboard_event_handle = Input::get_keyboard_event().bind(EVENT_CB(CameraController::on_keyboard_event_received));
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
        const float speed = 0.4f;
        glm::vec3 camera_pos = _camera->get_translation();
        glm::vec3 camera_front = _camera->get_front_vector();

        camera_pos += camera_front * mouse_event.offset_y * speed;
        _camera->set_translation(camera_pos);
    }
}

void CameraController::on_keyboard_event_received(KeyboardEvent keyboard_event)
{
    if (keyboard_event.action == KeyboardEvent::Action::PRESS)
    {
        _pressed_keys.insert((KeyCode)keyboard_event.key);
    }
    else if (keyboard_event.action == KeyboardEvent::Action::RELEASE)
    {
        _pressed_keys.erase((KeyCode)keyboard_event.key);
    }
}

void CameraController::tick(float dt)
{
    if (!_pressed_keys.empty())
    {
        const float speed = 0.15f;
        glm::vec3 camera_pos = _camera->get_translation();
        glm::vec3 camera_front = _camera->get_front_vector();
        glm::vec3 camera_right = _camera->get_right_vector();

        for (KeyCode key : _pressed_keys)
        {
            switch (key)
            {
                case KeyCode::ARROW_UP:
                    camera_pos += camera_front * speed;
                    break;
                case KeyCode::ARROW_DOWN:
                    camera_pos -= camera_front * speed;
                    break;
                case KeyCode::ARROW_LEFT:
                    camera_pos -= camera_right * speed;
                    break;
                case KeyCode::ARROW_RIGHT:
                    camera_pos += camera_right * speed;
                    break;
                default:
                    break;
            }
        }
        _camera->set_translation(camera_pos);
    }
}
