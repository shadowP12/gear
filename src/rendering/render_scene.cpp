#include "render_scene.h"
#include "entity/entity.h"
#include "render_context.h"
#include "render_system.h"
#include "vertex_factory.h"
#include "utils/render_utils.h"
#include <math/transform_util.h>

RenderScene::RenderScene()
{
}

RenderScene::~RenderScene()
{
    if(tlas)
    {
        ez_destroy_acceleration_structure(tlas);
    }
}

void RenderScene::predraw(RenderContext* ctx)
{
    EzBufferDesc buffer_desc{};
    buffer_desc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
    buffer_desc.size = sizeof(SceneInstanceData) * glm::max((size_t)1, g_scene->scene_insts.size());
    EzBuffer scene_ub = ctx->create_buffer("u_scene", buffer_desc, false);
    update_render_buffer(scene_ub, EZ_RESOURCE_STATE_SHADER_RESOURCE, sizeof(SceneInstanceData) * g_scene->scene_insts.size(), 0, (uint8_t*)g_scene->scene_insts.data());

    if (tlas_dirty)
    {
        tlas_dirty = false;

        if(tlas)
        {
            ez_destroy_acceleration_structure(tlas);
        }

        if(as_instances.size() > 0)
        {
            buffer_desc.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
            buffer_desc.memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
            buffer_desc.size = as_instances.size() * sizeof(VkAccelerationStructureInstanceKHR);
            EzBuffer as_instances_buffer = ctx->create_buffer("as_instances", buffer_desc, false);
            update_render_buffer(as_instances_buffer, EZ_RESOURCE_STATE_SHADER_RESOURCE, as_instances.size() * sizeof(VkAccelerationStructureInstanceKHR), 0, (uint8_t*)as_instances.data());

            EzAccelerationStructureBuildInfo tlas_info = {};
            tlas_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            tlas_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            tlas_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

            EzAccelerationStructureInstances instances;
            instances.flags = VK_GEOMETRY_TYPE_INSTANCES_KHR;
            instances.count = as_instances.size();
            instances.offset = 0;
            instances.instance_buffer = as_instances_buffer;
            tlas_info.geometry_set.instances.push_back(instances);

            ez_create_acceleration_structure(tlas_info, tlas);
            ez_build_acceleration_structure(tlas_info, tlas);
        }
    }
}

RenderScene* g_scene = nullptr;