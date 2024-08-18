#include "cluster_builder.h"
#include "render_system.h"
#include "render_context.h"
#include "render_shared_data.h"
#include "draw_command.h"
#include <core/bitops.h>
#include <core/memory.h>
#include <math/transform_util.h>

ClusterBuilder::ClusterBuilder()
{
    int element_max = MAX_CLUSTER_ELEMENT * ELEMENT_TYPE_MAX;
    _elements.resize(element_max);
}

ClusterBuilder::~ClusterBuilder()
{
    if (_cluster_buffer)
    {
        SAFE_DELETE(_cluster_buffer);
    }

    if (_cluster_render_buffer)
    {
        SAFE_DELETE(_cluster_render_buffer);
    }

    if (_state_buffer)
    {
        SAFE_DELETE(_state_buffer);
    }

    if (_elements_buffer)
    {
        SAFE_DELETE(_elements_buffer);
    }
}

void ClusterBuilder::begin(RenderContext* ctx, RenderView* view)
{
    _screen_size.x = ctx->viewport_size.z;
    _screen_size.y = ctx->viewport_size.w;
    _cluster_screen_size.x = (_screen_size.x - 1) / MAX_CLUSTER_SIZE + 1;
    _cluster_screen_size.y = (_screen_size.y - 1) / MAX_CLUSTER_SIZE + 1;

    _zn = view->zn;
    _zf = view->zf;
    _view_mat = view->view;
    _projection_mat = view->projection;
    _element_count = 0;

    _cluster_buffer_size = _cluster_screen_size.x * _cluster_screen_size.y * (MAX_CLUSTER_ELEMENT / 32 + 32) * ELEMENT_TYPE_MAX * 4;
    uint32_t element_tag_bits_size = MAX_CLUSTER_ELEMENT * ELEMENT_TYPE_MAX / 32;
    uint32_t element_tag_depth_bits_size = MAX_CLUSTER_ELEMENT * ELEMENT_TYPE_MAX;
    _cluster_render_buffer_size = _cluster_screen_size.x * _cluster_screen_size.y * (element_tag_bits_size + element_tag_depth_bits_size) * 4;
}

void ClusterBuilder::add_light(LightType type, int index, const glm::mat4& transform, float radius, float spot_aperture)
{
    RenderSharedData* shared_data = RenderSystem::get()->get_shared_data();
    ClusterElement& element = _elements[_element_count];
    glm::mat4 transform_cs = _view_mat * transform;
    glm::vec3 light_origin = TransformUtil::get_translation(transform_cs);

    if (type == LightType::Point)
    {
        radius *= shared_data->sphere_overfit;
        float depth = -light_origin.z;

        element.type = ELEMENT_TYPE_POINT_LIGHT;
        element.index = index;
        element.touches_near = (depth - radius) < _zn;
        element.touches_far = (depth + radius) > _zf;
        element.scale = glm::vec3(radius, radius, radius);
        element.transform = transform_cs;
    }
    else // type == LightType::Spot
    {
        radius *= shared_data->cone_overfit;
        float len = glm::tan(glm::radians(spot_aperture)) * radius;
        float depth = -light_origin.z;

        element.type = ELEMENT_TYPE_SPOT_LIGHT;
        element.index = index;
        element.touches_near = (depth - radius) < _zn;
        element.touches_far = (depth + radius) > _zf;

        if (spot_aperture > WIDE_SPOT_ANGLE_THRESHOLD_DEG)
        {
            element.scale = glm::vec3(radius, radius, radius);
            element.has_wide_spot_angle = true;
        }
        else
        {
            element.scale = glm::vec3(len * shared_data->cone_overfit, len * shared_data->cone_overfit, radius);
            element.has_wide_spot_angle = false;
        }
        element.transform = transform_cs;
    }

    _element_count++;
}

void ClusterBuilder::bake(RenderContext* ctx)
{
    DrawLabel draw_label("Prepare cluster buffer", DrawLabel::WHITE);

    if (!_cluster_buffer || _cluster_buffer->get_buffer()->size < _cluster_buffer_size)
    {
        if (_cluster_buffer)
        {
            SAFE_DELETE(_cluster_buffer);
        }

        _cluster_buffer = new UniformBuffer(_cluster_buffer_size);
    }
    _cluster_buffer->clear(_cluster_buffer_size);

    if (_element_count > 0)
    {
        if (!_cluster_render_buffer || _cluster_render_buffer->get_buffer()->size < _cluster_render_buffer_size)
        {
            if (_cluster_render_buffer)
            {
                SAFE_DELETE(_cluster_render_buffer);
            }

            _cluster_render_buffer = new UniformBuffer(_cluster_render_buffer_size);
        }
        _cluster_render_buffer->clear(_cluster_render_buffer_size);

        // Update state buffer
        if (!_state_buffer)
        {
            _state_buffer = new UniformBuffer(sizeof(ClusterBuilderState));
        }
        _state_buffer->write((uint8_t*)&_state, sizeof(ClusterBuilderState));

        if (!_elements_buffer || _elements_buffer->get_buffer()->size < sizeof(ClusterElement) * _element_count)
        {
            if (_elements_buffer)
            {
                SAFE_DELETE(_elements_buffer);
            }
            _elements_buffer = new UniformBuffer(sizeof(ClusterElement) * _element_count);
        }
        _elements_buffer->write((uint8_t*)_elements.data(), sizeof(ClusterElement) * _element_count);


    }
}

void ClusterBuilder::debug(RenderContext* ctx)
{

}