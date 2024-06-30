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
    renderable_count = 0;
    renderables.clear();
    instance_datas.clear();
}

void RenderScene::init_world()
{
}

void RenderScene::fill_draw_list(DrawCommandType type, int renderable_id)
{
    Renderable& renderable = renderables[renderable_id];
    glm::mat4& renderable_transform = instance_datas[renderable.scene_index].transform;
    glm::vec3 renderable_position = glm::vec3(renderable_transform[3][0], renderable_transform[3][1], renderable_transform[3][2]);
    MaterialProxy* material_proxy = renderable.material_proxy;

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
    draw_cmd->sort.vertex_factory_id = renderable.vertex_factory->get_type_id();
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

    renderable_count = 0;
    std::vector<Entity*>& mesh_entities = _world->get_mesh_entities();
    for (auto entity : mesh_entities)
    {
        CMesh* c_mesh = entity->get_component<CMesh>();
        Mesh* mesh = c_mesh->get_mesh();
        auto& surfaces = mesh->get_surfaces();
        for (auto& surface : surfaces)
        {
            if (renderable_count >= renderables.size())
            {
                renderables.emplace_back();
                instance_datas.emplace_back();
            }

            SceneInstanceData* instance_data = &instance_datas[renderable_count];
            instance_data->transform = entity->get_world_transform();

            Renderable* renderable = &renderables[renderable_count];
            renderable->scene_index = renderable_count;
            renderable->primitive_topology = surface->primitive_topology;
            renderable->vertex_factory = surface->vertex_factory;
            renderable->bounding_box = surface->bounding_box;
            renderable->material_proxy = surface->material->get_proxy();

            renderable_count++;
        }
    }

    if (renderable_count > 0)
    {
        if (scene_ub && scene_ub->get_buffer()->size < instance_datas.size() * sizeof(SceneInstanceData))
        {
            delete scene_ub;
        }

        if (!scene_ub)
        {
            uint32_t buffer_size = glm::min(64 * sizeof(SceneInstanceData), instance_datas.size() * sizeof(SceneInstanceData));
            scene_ub = new UniformBuffer(buffer_size);
        }

        scene_ub->write((uint8_t*)instance_datas.data(), renderable_count * sizeof(SceneInstanceData));
    }

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

void RenderScene::bind(int scene_idx)
{
    ez_bind_buffer(1, scene_ub->get_buffer(), sizeof(SceneInstanceData), scene_idx * sizeof(SceneInstanceData));
}