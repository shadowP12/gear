#include "render_scene.h"
#include "render_context.h"
#include "render_system.h"
#include "cluster_builder.h"
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
#include <math/transform_util.h>

RenderScene::RenderScene()
{
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
}

void RenderScene::init_world()
{
}

void RenderScene::fill_draw_list(DrawCommandType type, int renderable_id)
{
    Renderable* renderable = renderable_collector.get_item(renderable_id);
    glm::mat4& renderable_transform = scene_collector.get_item(renderable->scene_index)->transform;
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
    draw_cmd->distance = glm::distance(view[DISPLAY_VIEW].position, renderable_position);
    draw_cmd->sort.sort_key = 0;
    draw_cmd->sort.vertex_factory_id = renderable->vertex_factory->get_type_id();
    draw_cmd->sort.material_id = material_proxy->material_id;
}

void RenderScene::prepare(RenderContext* ctx)
{
    if (!_world)
        return;

    point_light_collector.clear();
    spot_light_collector.clear();
    dir_light_collector.clear();

    ClusterBuilder* cluster_builder = RenderSystem::get()->get_cluster_builder();
    cluster_builder->begin(ctx, &view[DISPLAY_VIEW]);

    std::vector<Entity*>& light_entities = _world->get_light_entities();
    for (auto entity : light_entities)
    {
        CLight* c_light = entity->get_component<CLight>();
        glm::mat4 light_transform = TransformUtil::remove_scale(entity->get_world_transform());
        glm::vec3 direction = entity->get_front_vector();
        glm::vec3 pos = entity->get_world_translation();

        if (c_light->get_light_type() == LightType::Direction)
        {
            dir_light_collector.add_item();
        }
        else if (c_light->get_light_type() == LightType::Point || c_light->get_light_type() == LightType::Spot)
        {
            int light_index = -1;
            OmniLightData* light_data = nullptr;
            if (c_light->get_light_type() == LightType::Point)
            {
                light_index = point_light_collector.add_item();
                light_data = point_light_collector.get_item(light_index);
            }
            else
            {
                light_index = spot_light_collector.add_item();
                light_data = spot_light_collector.get_item(light_index);
            }

            light_data->color = c_light->get_color();
            light_data->intensity = c_light->get_intensity();

            float radius = glm::max(0.001f, c_light->get_range());
            light_data->inv_radius = 1.0f / radius;

            light_data->position = pos;
            light_data->direction = direction;

            light_data->attenuation = c_light->get_attenuation();
            light_data->cone_attenuation = c_light->get_spot_attenuation();
            float spot_angle = c_light->get_spot_angle();
            light_data->cone_angle = glm::radians(spot_angle);

            cluster_builder->add_light(c_light->get_light_type(), light_index, light_transform, radius, spot_angle);
        }
    }
    if (point_light_collector.count > 0)
    {
        GpuBuffer* point_light_ub = ctx->create_ub("point_light_ub", point_light_collector.get_size());
        point_light_ub->write((uint8_t*)point_light_collector.get_data(), point_light_collector.get_size());
    }

    if (spot_light_collector.count > 0)
    {
        GpuBuffer* spot_light_ub = ctx->create_ub("spot_light_ub", spot_light_collector.get_size());
        spot_light_ub->write((uint8_t*)spot_light_collector.get_data(), spot_light_collector.get_size());
    }

    if (dir_light_collector.count > 0)
    {
        GpuBuffer* dir_light_ub = ctx->create_ub("dir_light_ub", dir_light_collector.get_size());
        dir_light_ub->write((uint8_t*)dir_light_collector.get_data(), dir_light_collector.get_size());
    }

    renderable_collector.clear();
    scene_collector.clear();
    std::vector<Entity*>& mesh_entities = _world->get_mesh_entities();
    for (auto entity : mesh_entities)
    {
        CMesh* c_mesh = entity->get_component<CMesh>();
        c_mesh->fill_renderables(&renderable_collector, &scene_collector);
    }
    if (scene_collector.count > 0)
    {
        GpuBuffer* scene_ub = ctx->create_ub("scene_ub", scene_collector.get_size());
        scene_ub->write((uint8_t*)scene_collector.get_data(), scene_collector.get_size());
    }

    for (int i = 0; i < DRAW_CMD_MAX; ++i)
    {
        draw_list[i].clear();
    }

    for (int i = 0; i < renderable_collector.count; ++i)
    {
        for (int j = 0; j < DRAW_CMD_MAX; ++j)
        {
            fill_draw_list((DrawCommandType)j, i);
        }
    }
    draw_list[DRAW_CMD_OPAQUE].sort();
    draw_list[DRAW_CMD_ALPHA].sort_by_depth();
}