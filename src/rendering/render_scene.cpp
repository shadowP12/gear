#include "render_scene.h"
#include "render_context.h"
#include "render_system.h"
#include "material_proxy.h"
#include "asset/level.h"
#include "asset/mesh.h"
#include "asset/material.h"
#include "asset/texture2d.h"
#include "entity/entity.h"
#include "entity/entity_notifications.h"
#include "entity/components/c_mesh.h"
#include "entity/components/c_camera.h"
#include "entity/components/c_light.h"
#include "entity/components/c_transform.h"

RenderScene::RenderScene()
{
    clear_scene();
}

RenderScene::~RenderScene()
{
    clear_scene();
}

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
    renderables.clear();

    _level_mappings.clear();
    _level->notify.unbind(_notify_handle);

    _reset_transform = true;
    _upload_transform_indices.clear();
    scene_transforms.clear();
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
            renderables.emplace_back();
            auto& renderable = renderables.back();

            CMesh* c_mesh = entity->get_component<CMesh>();
            CTransform* c_transform = entity->get_component<CTransform>();
            Mesh* mesh = c_mesh->get_mesh();
            auto& primitives = mesh->get_primitives();

            int scene_index = scene_transforms.size();
            SceneTransform scene_transform;
            scene_transform.transform = c_transform->get_world_transform();
            scene_transforms.push_back(scene_transform);

            renderable.scene_index = scene_index;
            renderable.primitives.clear();
            for (auto& primitive : primitives)
            {
                renderable.primitives.emplace_back();
                auto& render_primitive = renderable.primitives.back();
                render_primitive.primitive_topology = primitive->primitive_topology;
                render_primitive.vertex_factory = primitive->vertex_factory;
                render_primitive.vertex_count = primitive->vertex_count;
                render_primitive.index_count = primitive->index_count;
                render_primitive.index_type = primitive->index_type;
                render_primitive.vertex_buffer = primitive->vertex_buffer;
                render_primitive.index_buffer = primitive->index_buffer;
                render_primitive.bounding_box = primitive->bounding_box;
                render_primitive.material_id = primitive->material->get_material_id();
            }
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
            if (c_camera->get_usage() & CameraUsage::CAMERA_USAGE_MAIN)
            {
                init_view_func(0);
            }
            if (c_camera->get_usage() & CameraUsage::CAMERA_USAGE_DISPLAY)
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
        if (c_camera->get_usage() & CameraUsage::CAMERA_USAGE_MAIN)
        {
            change_view_func(0);
        }
        if (c_camera->get_usage() & CameraUsage::CAMERA_USAGE_DISPLAY)
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
        if (c_camera->get_usage() & CameraUsage::CAMERA_USAGE_MAIN)
        {
            change_view_func(0);
        }
        if (c_camera->get_usage() & CameraUsage::CAMERA_USAGE_DISPLAY)
        {
            change_view_func(1);
        }
    }
}

void RenderScene::fill_draw_list(DrawCommandType type, int renderable_id)
{
    Renderable& renderable = renderables[renderable_id];
    glm::mat4& renderable_transform = scene_transforms[renderable.scene_index].transform;
    glm::vec3 renderable_pos = glm::vec3(renderable_transform[3][0], renderable_transform[3][1], renderable_transform[3][2]);
    MaterialProxyPool* material_proxy_pool = RenderSystem::get()->get_material_proxy_pool();
    for (int i = 0; i < renderable.primitives.size(); ++i)
    {
        auto& primitive = renderable.primitives[i];
        MaterialProxy* material_proxy = material_proxy_pool->get_proxy(primitive.material_id);

        DrawCommand* draw_cmd = nullptr;
        if (type == DRAW_CMD_OPAQUE && material_proxy->alpha_mode == MaterialAlphaMode::Opaque)
        {
            draw_cmd = draw_list[type].add_element();
        }
        else if (type == DRAW_CMD_ALPHA && material_proxy->alpha_mode != MaterialAlphaMode::Opaque)
        {
            draw_cmd = draw_list[type].add_element();
        }

        if (!draw_cmd)
            continue;

        draw_cmd->renderable = renderable_id;
        draw_cmd->primitive = i;
        draw_cmd->distance = glm::distance(view[1].position, renderable_pos);
        draw_cmd->sort.sort_key = 0;
        draw_cmd->sort.vertex_factory_id = primitive.vertex_factory;
        draw_cmd->sort.material_id = primitive.material_id;
    }
}

void RenderScene::prepare(RenderContext* ctx)
{
    if (_reset_transform || !scene_ub)
    {
        if (!scene_ub)
        {
            uint32_t buffer_size = glm::min(64 * sizeof(SceneTransform), scene_transforms.size() * sizeof(SceneTransform));
            scene_ub = std::make_shared<UniformBuffer>(buffer_size);
        }

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
    _reset_transform = false;
    _upload_transform_indices.clear();

    for (int i = 0; i < DRAW_CMD_MAX; ++i)
    {
        draw_list[i].clear();
    }

    for (int i = 0; i < renderables.size(); ++i)
    {
        auto& renderable = renderables[i];
        if (renderable.scene_index < 0)
            continue;

        for (int j = 0; j < DRAW_CMD_MAX; ++j)
        {
            fill_draw_list((DrawCommandType)j, i);
        }
    }
    draw_list[DRAW_CMD_OPAQUE].sort();
    draw_list[DRAW_CMD_ALPHA].sort_by_depth();
}