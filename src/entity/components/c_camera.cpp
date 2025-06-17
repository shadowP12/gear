#include "c_camera.h"
#include "entity/entity.h"
#include "rendering/render_scene.h"
#include "viewport.h"
#include "world.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

CCamera::CCamera(Entity* entity)
    : CRender(entity) {}

CCamera::~CCamera() {}

void CCamera::serialize(SerializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.object([&]() {
        ctx.field("class_name",get_class_name());
        ctx.field("near", _near);
        ctx.field("far", _far);
        ctx.field("fov", _fov);
        ctx.field("exposure", _exposure);
        ctx.field("usage", _usage);
        ctx.field("mode", _mode);
    });
}

void CCamera::deserialize(DeserializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.field("near", _near);
    ctx.field("far", _far);
    ctx.field("fov", _fov);
    ctx.field("exposure", _exposure);
    ctx.field("usage", _usage);
    ctx.field("mode", _mode);
}

glm::mat4 CCamera::get_proj_matrix()
{
    World* world = _entity->get_world();
    if (!world)
        return glm::mat4(1.0f);

    Viewport* vp = world->get_viewport();
    if (!vp)
        return glm::mat4(1.0f);

    float aspect = vp->get_aspect();
    if (_mode == ProjectionMode::Perspective)
    {
        glm::mat4 proj_matrix;
        proj_matrix = glm::perspective(glm::radians(_fov), aspect, _near, _far);
        // Reverse y
        proj_matrix[1][1] *= -1;
        return proj_matrix;
    }
    else if (_mode == ProjectionMode::Ortho)
    {
        glm::mat4 proj_matrix;
        float size = 1.0;
        proj_matrix = glm::ortho(-size / 2, +size / 2, -size / aspect / 2, +size / aspect / 2, _near, _far);
        return proj_matrix;
    }
    return glm::mat4(1.0f);
}

void CCamera::set_perspective()
{
    make_render_dirty();
    _mode = ProjectionMode::Perspective;
}

void CCamera::set_orthogonal()
{
    make_render_dirty();
    _mode = ProjectionMode::Ortho;
}

void CCamera::set_near(float n)
{
    make_render_dirty();
    _near = n;
}

void CCamera::set_far(float f)
{
    make_render_dirty();
    _far = f;
}

void CCamera::set_uasge(ViewUsageFlags usage)
{
    make_render_dirty();
    _usage = usage;
}

void CCamera::set_exposure(float exposure)
{
    make_render_dirty();
    _exposure = exposure;
}

void CCamera::predraw()
{
    auto change_view_func = [&](int view_id) {
        g_scene->view[view_id].model = TransformUtil::remove_scale(_entity->get_world_transform());
        g_scene->view[view_id].view = glm::inverse(g_scene->view[view_id].model);
        g_scene->view[view_id].proj = get_proj_matrix();
        g_scene->view[view_id].inv_proj = glm::inverse(get_proj_matrix());
        g_scene->view[view_id].position = _entity->get_world_translation();
        g_scene->view[view_id].direction = glm::normalize(_entity->get_front_vector());

        g_scene->view[view_id].zn = get_near();
        g_scene->view[view_id].zf = get_far();
        g_scene->view[view_id].exposure = get_exposure();

        g_scene->view[view_id].proj_model = get_proj_mode();
    };

    if (enum_has_flags(_usage, ViewUsageFlags::Main))
    {
        change_view_func(MAIN_VIEW);
    }

    if (enum_has_flags(_usage, ViewUsageFlags::Display))
    {
        change_view_func(DISPLAY_VIEW);
    }
}

REGISTER_ENTITY_COMPONENT(CCamera)