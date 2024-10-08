#pragma once

#include "render_resources.h"
#include "render_context.h"
#include "render_constants.h"
#include <math/bounding_box.h>
#include <math/plane.h>
#include <rhi/ez_vulkan.h>
#include <rhi/rhi_shader_mgr.h>

#define MAX_CLUSTER_SIZE 32
#define MAX_CLUSTER_ELEMENT 32
#define WIDE_SPOT_ANGLE_THRESHOLD_DEG 60.0f

struct ClusterElement
{
    uint32_t type;
    uint32_t index;
    uint32_t pad0;
    uint32_t pad1;
    glm::mat4 transform;
    glm::vec3 scale;
    uint32_t has_wide_spot_angle;
};

struct ClusterBuilderState
{
    glm::mat4 projection;
    float inv_z_far;
    uint32_t screen_to_clusters_shift;
    uint32_t cluster_screen_width;
    uint32_t cluster_data_size;
    uint32_t cluster_depth_offset;
    uint32_t cluster_helper_offset;
    uint32_t pad0;
    uint32_t pad1;
};

class ClusterBuilder
{
public:
    enum ElementType
    {
        ELEMENT_TYPE_POINT_LIGHT,
        ELEMENT_TYPE_SPOT_LIGHT,
        ELEMENT_TYPE_MAX,
    };

public:
    ClusterBuilder();

    ~ClusterBuilder();

    void begin(RenderContext* ctx, RenderView* view);

    void add_light(LightType type, int index, const glm::mat4& transform, float radius, float spot_aperture);

    void bake(RenderContext* ctx);

    void debug(RenderContext* ctx);

private:
    float _zn;
    float _zf;
    bool _orthogonal;
    glm::mat4 _view_mat;
    glm::mat4 _projection_mat;
    glm::ivec2 _screen_size;
    glm::ivec2 _cluster_screen_size;
    ClusterBuilderState _state;
    int _element_count = 0;
    std::vector<ClusterElement> _elements;
    int _max_element_count = 0;
    int _cluster_buffer_size = 0;
    int _cluster_render_buffer_size = 0;
};