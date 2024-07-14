#include "render_scene.h"
#include "render_context.h"
#include "render_system.h"
#include "material_proxy.h"
#include "vertex_factory.h"
#include "world.h"
#include "asset/level.h"
#include "asset/mesh.h"
#include "asset/material.h"
#include "asset/texture2d.h"
#include "entity/entity.h"
#include "entity/entity_notifications.h"
#include "entity/components/c_mesh.h"
#include "entity/components/c_camera.h"
#include "entity/components/c_light.h"

RenderScene::RenderScene()
{
    clear_world();
}

RenderScene::~RenderScene()
{
    clear_world();
}

void RenderScene::set_world(World* world)
{
    if (_world == world)
        return;

    if (_world)
    {
        clear_world();
    }

    _world = world;
    if (_world)
    {
        init_world();
    }
}

void RenderScene::clear_world()
{
    if (scene_ub)
    {
        SAFE_DELETE(scene_ub);
    }
    collector.clear();
}

void RenderScene::init_world()
{
}

void RenderScene::fill_draw_list(DrawCommandType type, int renderable_id)
{
    Renderable* renderable = collector.get_renderable(renderable_id);
    glm::mat4& renderable_transform = collector.instance_datas[renderable->scene_index].transform;
    glm::vec3 renderable_position = glm::vec3(renderable_transform[3][0], renderable_transform[3][1], renderable_transform[3][2]);
    MaterialProxy* material_proxy = renderable->material_proxy;

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
        return;

    draw_cmd->renderable = renderable_id;
    draw_cmd->distance = glm::distance(view[1].position, renderable_position);
    draw_cmd->sort.sort_key = 0;
    draw_cmd->sort.vertex_factory_id = renderable->vertex_factory->get_type_id();
    draw_cmd->sort.material_id = material_proxy->material_id;
}

void RenderScene::prepare(RenderContext* ctx)
{
    if (!_world)
        return;

    std::vector<Entity*>& camera_entities = _world->get_camera_entities();
    for (auto entity : camera_entities)
    {
        CCamera* c_camera = entity->get_component<CCamera>();

        auto change_view_func = [&](int view_id)
        {
            view[view_id].model = entity->get_world_transform();
            view[view_id].view = glm::inverse(view[view_id].model);
            view[view_id].position = entity->get_translation();
            view[view_id].view_direction = entity->get_front_vector();

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

    collector.clear();
    std::vector<Entity*>& mesh_entities = _world->get_mesh_entities();
    for (auto entity : mesh_entities)
    {
        CMesh* c_mesh = entity->get_component<CMesh>();
        c_mesh->fill_renderables(&collector);
    }

    if (collector.renderable_count > 0)
    {
        if (scene_ub && scene_ub->get_buffer()->size < collector.instance_datas.size() * sizeof(SceneInstanceData))
        {
            SAFE_DELETE(scene_ub);
        }

        if (!scene_ub)
        {
            uint32_t buffer_size = glm::min(64 * sizeof(SceneInstanceData), collector.instance_datas.size() * sizeof(SceneInstanceData));
            scene_ub = new UniformBuffer(buffer_size);
        }

        scene_ub->write((uint8_t*)collector.instance_datas.data(), collector.renderable_count * sizeof(SceneInstanceData));
    }

    for (int i = 0; i < DRAW_CMD_MAX; ++i)
    {
        draw_list[i].clear();
    }

    for (int i = 0; i < collector.renderable_count; ++i)
    {
        for (int j = 0; j < DRAW_CMD_MAX; ++j)
        {
            fill_draw_list((DrawCommandType)j, i);
        }
    }
    draw_list[DRAW_CMD_OPAQUE].sort();
    draw_list[DRAW_CMD_ALPHA].sort_by_depth();
}

void RenderScene::bind(int scene_idx)
{
    ez_bind_buffer(1, scene_ub->get_buffer(), sizeof(SceneInstanceData), scene_idx * sizeof(SceneInstanceData));
}