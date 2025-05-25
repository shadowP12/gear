#version 450
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"

layout(location = 0) in vec4 vertex_world_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;

layout(location = 0) out vec4 frag_color;

#define USING_FRAME_UNIFORMS 0
#include "uniforms.glsl"

// 2 - 5
#define USING_CLUSTER_LIGHTING 2
#include "light_inc.glsl"

// 6 - 7
#define USING_SHADOW_INFO 6
#define USING_SHADOW_MAP 7
#include "shadow_inc.glsl"

layout(std140, binding = 8) uniform Params
{
    uvec2 screen_size;
} u_params;

uint get_cluster_index(vec4 frag_coord)
{
    float z_near = u_frame.z_near_far.x;
    float z_far = u_frame.z_near_far.y;

    float depth = frag_coord.z;
    depth = z_near * z_far / (z_far + depth * (z_near - z_far)); // Linearize depth
    uint z_tile = uint(log2(abs(depth) / z_near) / log2(z_far / z_near) * float(u_frame.cluster_size.z));

    vec2 tile_size = u_params.screen_size / u_frame.cluster_size.xy;
    uvec3 tile = uvec3(frag_coord.xy / tile_size, z_tile);
    uint tile_index = tile.x + (tile.y * u_frame.cluster_size.x) + (tile.z * u_frame.cluster_size.x * u_frame.cluster_size.y);
    return tile_index;
}

vec3 surface_shading()
{
    vec3 base_color = vec3(1.0, 1.0, 1.0);
    return base_color / 3.1415926;
}

void main()
{
    vec3 V = normalize(u_frame.view_direction.xyz);
    vec3 N = normalize(vertex_normal);
    float NoV = clamp(dot(N, V), 0.0, 1.0);
    float exposure = u_frame.exposure;
    float noise = quick_hash(gl_FragCoord.xy) * 0.05;//  Reduce color banding
    vec4 final_color = vec4(0.0, 0.0, 0.0, 1.0);

    uint cluster_index = get_cluster_index(gl_FragCoord);

    uint point_lit_bits = s_clusters.data[cluster_index].lit_bits.x;
    while (point_lit_bits != 0)
    {
        uint bit = findLSB(point_lit_bits);
        point_lit_bits &= uint(~(1 << bit));

        PunctualLight lit = u_point_lits.data[bit];
        vec3 pos_to_lit = lit.position - vertex_world_position.xyz;
        vec3 L = normalize(pos_to_lit);
        vec3 H = normalize(V + L);
        float NoL = clamp(dot(N, L), 0.0, 1.0);
        float NoH = clamp(dot(N, H), 0.0, 1.0);
        float LoH = clamp(dot(L, H), 0.0, 1.0);
        float distance = length(pos_to_lit);
        float attenuation = get_distance_attenuation(distance + noise, lit.inv_radius);

        final_color.xyz += surface_shading() * lit.color * lit.intensity * exposure * attenuation;
    }

    uint spot_lit_bits = s_clusters.data[cluster_index].lit_bits.y;
    while (spot_lit_bits != 0)
    {
        uint bit = findLSB(spot_lit_bits);
        spot_lit_bits &= uint(~(1 << bit));

        PunctualLight lit = u_spot_lits.data[bit];
        vec3 pos_to_lit = lit.position - vertex_world_position.xyz;
        vec3 L = normalize(pos_to_lit);
        vec3 H = normalize(V + L);
        float NoL = clamp(dot(N, L), 0.0, 1.0);
        float NoH = clamp(dot(N, H), 0.0, 1.0);
        float LoH = clamp(dot(L, H), 0.0, 1.0);
        float distance = length(pos_to_lit);
        float attenuation = get_distance_attenuation(distance + noise, lit.inv_radius);
        attenuation *= get_angle_attenuation(L, lit.direction, lit.cone_angle, lit.inner_angle);

        final_color.xyz += surface_shading() * lit.color * lit.intensity * exposure * attenuation;
    }

    if (u_frame.has_sun)
    {
        DirectionLight lit = u_dir_lits.data[0];
        vec3 L = normalize(-lit.direction);
        float NoL = clamp(dot(N, L), 0.0, 1.0);

        float visibility = 1.0;
        if (lit.has_shadow)
        {
            PerShadowInfo shadow_info = u_shadow_infos.data[lit.shadow];
            vec3 view_position = (u_frame.view_matrix * vertex_world_position).xyz;
            bvec4 greater_z = greaterThan(vec4(abs(view_position.z)), shadow_info.cascade_splits);
            uint cascade = clamp(uint(dot(vec4(greater_z), vec4(1.0))), 0u, SHADOW_CASCADE_COUNT - 1u);

            mat4 xx = shadow_info.light_matrixs[cascade];
            vec4 light_space_position = (xx * vertex_world_position);
            vec3 shadow_position = light_space_position.xyz / light_space_position.w;
            shadow_position.xy = shadow_position.xy * 0.5 + 0.5;
            float bias = 0.005;
            #if defined(SHADOW_SIMPLE)
            visibility = sampler_shadow(cascade, shadow_position, bias);
            #elif defined(SHADOW_PCF)
            visibility = sampler_shadow_pcf(cascade, shadow_position, bias);
            #elif defined(SHADOW_OPTIMIZED_PCF)
            visibility = sampler_shadow_pcf_optimized(cascade, shadow_position, bias);
            #elif defined(SHADOW_RANDOM_DISC_PCF)
            visibility = sampler_shadow_pcf_random_disc(cascade, shadow_position, bias);
            #endif
        }

        final_color.xyz += surface_shading() * lit.color * lit.intensity * exposure * NoL * visibility;
    }

    frag_color = final_color;
}