#include "render_scene.h"
#include "render_context.h"
#include "asset/level.h"
#include "entity/entity.h"
#include "entity/entity_notifications.h"
#include "entity/components/c_mesh.h"
#include "entity/components/c_camera.h"
#include "entity/components/c_light.h"
#include "entity/components/c_transform.h"

RenderScene::RenderScene()
{}

RenderScene::~RenderScene()
{}

void RenderScene::set_level(Level* level)
{
    if (_level == level)
        return;

    if (_level)
        clear_scene();

    _level = level;
    init_scene();
    _notify_handle = _level->notify.bind(EVENT_CB(RenderScene::notify_received));
}

void RenderScene::clear_scene()
{
    _level->notify.unbind(_notify_handle);
    renderables.clear();
    scene_transforms.clear();
    _upload_transform_indices.clear();
    _level_mappings.clear();
}

void RenderScene::init_scene()
{
    std::vector<Entity*>& entities = _level->get_entities();
    for (auto entity : entities)
    {
        LevelMapping mapping;

        // Renderable
        if (entity->has_component<CMesh>())
        {
            int rb_id = renderables.size();
            mapping.rb = rb_id;
            CMesh* c_mesh = entity->get_component<CMesh>();
            CTransform* c_transform = entity->get_component<CTransform>();

            SceneTransform scene_transform;
            scene_transform.transform = c_transform->get_world_transform();
            scene_transforms.push_back(scene_transform);

            Renderable renderable;
            renderable.scene_index = rb_id;
            renderables.push_back(renderable);
        }

        // RenderView
        if (entity->has_component<CCamera>())
        {
            CCamera* c_camera = entity->get_component<CCamera>();
            CTransform* c_transform = entity->get_component<CTransform>();

            mapping.view = 0;
            auto init_view_func = [&](int view_id) {
                view[view_id].model = c_transform->get_world_transform();
                view[view_id].view = glm::inverse(view[view_id].model);
                view[view_id].position = c_transform->get_position();
                view[view_id].view_direction = c_transform->get_front_vector();
                view[view_id].zn = c_camera->get_near();
                view[view_id].zf = c_camera->get_far();
                view[view_id].projection = c_camera->get_proj_matrix();
                view[view_id].ev100 = std::log2((c_camera->get_aperture() * c_camera->get_aperture()) / c_camera->get_shutter_speed() * 100.0f / c_camera->get_sensitivity());
                view[view_id].exposure = 1.0f / (1.2f * std::pow(2.0, view[view_id].ev100));
            };
            if (c_camera->get_usage() & CameraUsage::MAIN)
            {
                init_view_func(0);
            }
            if (c_camera->get_usage() & CameraUsage::DISPLAY)
            {
                init_view_func(1);
            }
        }

        _level_mappings.push_back(mapping);
    }
}

void RenderScene::notify_received(int what, int id)
{
    switch (what)
    {
        case NOTIFY_TRANSFORM_CHANGED:
            transform_changed(id);
            break;
        case NOTIFY_CAMERA_CHANGED:
            camera_changed(id);
            break;
        default:
            break;
    }
}

void RenderScene::transform_changed(int id)
{
    if (_level_mappings[id].rb >= 0)
    {
        int rb_id = _level_mappings[id].rb;
        CTransform* c_transform = _level->get_entity(id)->get_component<CTransform>();
        scene_transforms[rb_id].transform = c_transform->get_world_transform();
        _upload_transform_indices.push_back(rb_id);
    }
    else if (_level_mappings[id].view >= 0)
    {
        CCamera* c_camera = _level->get_entity(id)->get_component<CCamera>();
        CTransform* c_transform = _level->get_entity(id)->get_component<CTransform>();

        auto change_view_func = [&](int view_id) {
            view[view_id].model = c_transform->get_world_transform();
            view[view_id].view = glm::inverse(view[view_id].model);
            view[view_id].position = c_transform->get_position();
            view[view_id].view_direction = c_transform->get_front_vector();
        };
        if (c_camera->get_usage() & CameraUsage::MAIN)
        {
            change_view_func(0);
        }
        if (c_camera->get_usage() & CameraUsage::DISPLAY)
        {
            change_view_func(1);
        }
    }
}

void RenderScene::camera_changed(int id)
{
    if (_level_mappings[id].view >= 0)
    {
        CCamera* c_camera = _level->get_entity(id)->get_component<CCamera>();

        auto change_view_func = [&](int view_id) {
            view[view_id].zn = c_camera->get_near();
            view[view_id].zf = c_camera->get_far();
            view[view_id].projection = c_camera->get_proj_matrix();
            view[view_id].ev100 = std::log2((c_camera->get_aperture() * c_camera->get_aperture()) / c_camera->get_shutter_speed() * 100.0f / c_camera->get_sensitivity());
            view[view_id].exposure = 1.0f / (1.2f * std::pow(2.0, view[view_id].ev100));
        };
        if (c_camera->get_usage() & CameraUsage::MAIN)
        {
            change_view_func(0);
        }
        if (c_camera->get_usage() & CameraUsage::DISPLAY)
        {
            change_view_func(1);
        }
    }
}

void RenderScene::prepare(RenderContext* ctx)
{
    bool is_new;
    UniformBuffer* scene_ub = ctx->find_ub(SCENE_TRANSFORMS_NAME, scene_transforms.size() * sizeof(SceneTransform), is_new);
    if (is_new)
    {
        scene_ub->write((uint8_t*)scene_transforms.data(), scene_transforms.size() * sizeof(SceneTransform));
    }
    else
    {
        for (int i = 0; i < _upload_transform_indices.size(); ++i)
        {
            int upload_idx = _upload_transform_indices[i];
            scene_ub->write((uint8_t*)&scene_transforms[upload_idx], sizeof(SceneTransform), upload_idx * sizeof(SceneTransform));
        }
    }
    _upload_transform_indices.clear();
}