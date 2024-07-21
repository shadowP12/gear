#include "cluster_builder.h"
#include <core/bitops.h>

ClusterBuilder::ClusterBuilder()
{
}

ClusterBuilder::~ClusterBuilder()
{
}

void ClusterBuilder::begin(RenderContext* ctx, RenderView* view)
{
    _screen_size.x = ctx->viewport_size.z;
    _screen_size.y = ctx->viewport_size.w;
    _cluster_screen_size.x = (_screen_size.x - 1) / _cluster_size + 1;
    _cluster_screen_size.y = (_screen_size.y - 1) / _cluster_size + 1;

    _zn = view->zn;
    _zf = view->zf;
    _view_mat = view->view;
    _projection_mat = view->projection;
    _element_count = 0;
}

void ClusterBuilder::add_light(LightType type, const glm::mat4& transform, float radius, float spot_aperture)
{

}

void ClusterBuilder::render(RenderContext* ctx)
{

}

void ClusterBuilder::debug(RenderContext* ctx)
{

}