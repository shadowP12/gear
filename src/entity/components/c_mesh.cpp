#include "c_mesh.h"
#include "entity/entity.h"
#include "asset/mesh.h"
#include "asset/material.h"
#include "asset/asset_manager.h"
#include "rendering/renderable.h"
#include "rendering/collector/renderable_collector.h"

CMesh::CMesh(Entity* entity)
    : EntityComponent(entity)
{
}

CMesh::~CMesh()
{
}

void CMesh::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("class_name");
    writer.String(get_class_name().c_str());
    writer.Key("mesh");
    writer.String(_mesh->get_asset_path().c_str());
    writer.EndObject();
}

void CMesh::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    _mesh = AssetManager::get()->load<Mesh>(value["mesh"].GetString());
}

void CMesh::set_mesh(Mesh* mesh) {
    _mesh = mesh;
}

Mesh* CMesh::get_mesh() {
    return _mesh;
}

void CMesh::fill_renderables(RenderableCollector* collector, SceneInstanceCollector* scene_collector)
{
    if (!_mesh)
    {
        return;
    }

    auto& surfaces = _mesh->get_surfaces();
    for (auto& surface : surfaces)
    {
        int scene_idx = scene_collector->add_item();
        SceneInstanceData* instance_data = scene_collector->get_item(scene_idx);
        instance_data->transform = _entity->get_world_transform();

        int idx = collector->add_item();
        Renderable* renderable = collector->get_item(idx);
        renderable->scene_index = scene_idx;
        renderable->primitive_topology = surface->primitive_topology;
        renderable->vertex_factory = surface->vertex_factory;
        renderable->bounding_box = surface->bounding_box;
        renderable->material_proxy = surface->material->get_proxy();
    }
}

REGISTER_ENTITY_COMPONENT(CMesh)