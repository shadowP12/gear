#include "c_mesh.h"
#include "entity/entity.h"
#include "asset/mesh_asset.h"
#include "asset/material_asset.h"
#include "asset/asset_manager.h"
#include "rendering/renderable.h"
#include "rendering/render_scene.h"

CMesh::CMesh(Entity* entity)
    : CRender(entity)
{
    _scene_inst = INVALID_OBJECT_S;
}

CMesh::~CMesh()
{
    if (_scene_inst != INVALID_OBJECT_S)
    {
        g_scene->scene_insts.remove(_scene_inst);
        _scene_inst = INVALID_OBJECT_S;
    }
    destroy_renderables();
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
    set_mesh(AssetManager::get()->load<MeshAsset>(value["mesh"].GetString()));
}

void CMesh::set_mesh(MeshAsset* mesh)
{
    make_render_dirty();
    destroy_renderables();
    _mesh = mesh;
}

MeshAsset* CMesh::get_mesh()
{
    return _mesh;
}

void CMesh::make_mesh_render_dirty(MeshRnderDirtyFlag flag)
{
    _render_dirty_flags |= flag;
    make_render_dirty();
}

void CMesh::destroy_renderables()
{
    for (int i = 0; i < _renderables.size(); ++i)
    {
        g_scene->renderables.remove(_renderables[i]);
    }
    _renderables.clear();
    _vertex_factorys.clear();
}

void CMesh::predraw()
{
    if ( _scene_inst == INVALID_OBJECT_S )
    {
        _scene_inst = g_scene->scene_insts.add();
    }

    uint32_t scene_inst_index = g_scene->scene_insts.get_index(_scene_inst);
    auto scene_inst_obj = g_scene->scene_insts.get_obj(_scene_inst);
    scene_inst_obj->transform = _entity->get_world_transform();

    if (_mesh && _renderables.size() == 0)
    {
        auto& surfaces = _mesh->get_surfaces();
        for (int i = 0; i < surfaces.size(); ++i)
        {
            auto surface = surfaces[i];
            _vertex_factorys.emplace_back();
            auto& vertex_factory = _vertex_factorys.back();

            ez_set_vertex_binding(vertex_factory.layout, 0, 12);
            ez_set_vertex_attrib(vertex_factory.layout, 0, 0, VK_FORMAT_R32G32B32_SFLOAT);

            ez_set_vertex_binding(vertex_factory.layout, 1, 12);
            ez_set_vertex_attrib(vertex_factory.layout, 1, 1, VK_FORMAT_R32G32B32_SFLOAT);

            ez_set_vertex_binding(vertex_factory.layout, 2, 12);
            ez_set_vertex_attrib(vertex_factory.layout, 2, 2, VK_FORMAT_R32G32B32_SFLOAT);

            ez_set_vertex_binding(vertex_factory.layout, 3, 8);
            ez_set_vertex_attrib(vertex_factory.layout, 3, 3, VK_FORMAT_R32G32_SFLOAT);

            vertex_factory.vertex_count = surface->vertex_count;
            vertex_factory.vertex_buffer_count = surface->vertex_buffer_count;
            for (int j = 0; j < surface->vertex_buffer_count; ++j)
            {
                vertex_factory.vertex_buffers[j] = surface->vertex_buffers[j];
            }
            vertex_factory.index_count = surface->index_count;
            vertex_factory.index_type = surface->index_type;
            vertex_factory.index_buffer = surface->index_buffer;
            vertex_factory.prim_topo = surface->primitive_topology;

            auto& programs = surface->material->get_programs();
            for (auto& programs_iter : programs)
            {
                uint32_t renderable_hanele = g_scene->renderables.add();
                Renderable* renderable = g_scene->renderables.get_obj(renderable_hanele);
                renderable->draw_type = programs_iter.first;
                renderable->program = programs_iter.second;
                renderable->vertex_factory = &vertex_factory;
                renderable->local_bounding_box = surface->bounding_box;

                _renderables.push_back(renderable_hanele);
            }
        }
    }

    for (int i = 0; i < _renderables.size(); ++i)
    {
        Renderable* renderable = g_scene->renderables.get_obj(_renderables[i]);
        renderable->scene_index = scene_inst_index;
        renderable->transform = scene_inst_obj->transform;
        renderable->bounding_box = renderable->local_bounding_box.transform(renderable->transform);
    }
}

REGISTER_ENTITY_COMPONENT(CMesh)