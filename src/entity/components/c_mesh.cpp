#include "c_mesh.h"
#include "asset/asset_manager.h"
#include "asset/material_asset.h"
#include "asset/mesh_asset.h"
#include "asset/sdf_asset.h"
#include "entity/entity.h"
#include "rendering/render_scene.h"
#include "rendering/renderable.h"

CMesh::CMesh(Entity* entity)
    : CRender(entity)
{
    _render_data_inited = false;
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

void CMesh::serialize(SerializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.object([&]() {
        ctx.field("class_name", get_class_name());
        ctx.field("mesh", _mesh->get_asset_path());
    });
}

void CMesh::deserialize(DeserializationContext& ctx, BinaryStream& bin_stream)
{
    std::string mesh_url;
    ctx.field("mesh", mesh_url);
    set_mesh(AssetManager::get()->load<MeshAsset>(mesh_url));
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

    for (int i = 0; i < _sdf_objects.size(); ++i)
    {
        g_scene->sdf_objects.remove(_sdf_objects[i]);
    }

    g_scene->tlas_dirty = true;
    for (int i = 0; i < _as_instances.size(); ++i)
    {
        g_scene->as_instances.remove(_as_instances[i]);
    }

    _renderables.clear();
    _sdf_objects.clear();
    _vertex_factorys.clear();
    _render_data_inited = false;
}

void CMesh::predraw()
{
    if (_scene_inst == INVALID_OBJECT_S)
    {
        _scene_inst = g_scene->scene_insts.add();
    }

    uint32_t scene_inst_index = g_scene->scene_insts.get_index(_scene_inst);
    auto scene_inst_obj = g_scene->scene_insts.get_obj(_scene_inst);
    scene_inst_obj->transform = _entity->get_world_transform();

    if (_mesh && !_render_data_inited)
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
                uint32_t renderable_handle = g_scene->renderables.add();
                Renderable* renderable = g_scene->renderables.get_obj(renderable_handle);
                renderable->draw_type = programs_iter.first;
                renderable->program = programs_iter.second;
                renderable->vertex_factory = &vertex_factory;
                renderable->local_bounding_box = surface->bounding_box;

                _renderables.push_back(renderable_handle);
            }

            if (surface->sdf)
            {
                uint32_t sdf_handle = g_scene->sdf_objects.add();
                SDFObject* sdf_object = g_scene->sdf_objects.get_obj(sdf_handle);
                sdf_object->texture = surface->sdf->get_texture();
                sdf_object->local_to_uvw_add = surface->sdf->get_local_to_uvw_add();
                sdf_object->local_to_uvw_mul = surface->sdf->get_local_to_uvw_mul();
                sdf_object->bounds = surface->sdf->get_bounds();
                sdf_object->resolution = surface->sdf->get_resolution();

                _sdf_objects.push_back(sdf_handle);
            }

            if (surface->blas)
            {
                uint32_t as_inst_handle = g_scene->as_instances.add();
                VkAccelerationStructureInstanceKHR* as_inst = g_scene->as_instances.get_obj(as_inst_handle);
                as_inst->instanceCustomIndex = 0;
                as_inst->instanceShaderBindingTableRecordOffset = 0;
                as_inst->flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
                as_inst->mask = 0xFF;
                as_inst->accelerationStructureReference = surface->blas->buffer->address;

                _as_instances.push_back(as_inst_handle);
            }
        }

        _render_data_inited = true;
    }

    for (int i = 0; i < _renderables.size(); ++i)
    {
        Renderable* renderable = g_scene->renderables.get_obj(_renderables[i]);
        renderable->scene_index = scene_inst_index;
        renderable->transform = scene_inst_obj->transform;
        renderable->bounding_box = renderable->local_bounding_box.transform(renderable->transform);
    }

    g_scene->tlas_dirty = true;
    for (int i = 0; i < _as_instances.size(); ++i)
    {
        VkAccelerationStructureInstanceKHR* as_inst = g_scene->as_instances.get_obj(_as_instances[i]);
        auto transposed = glm::transpose(scene_inst_obj->transform);
        std::memcpy(&as_inst->transform, &transposed, sizeof(VkTransformMatrixKHR));
    }
}

REGISTER_ENTITY_COMPONENT(CMesh)