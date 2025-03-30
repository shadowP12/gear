#version 450
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

#define USING_FRAME_UNIFORMS 0
#include "uniforms.glsl"

layout(std140, binding = 1) uniform Params
{
    uvec2 geom_count;
} u_params;

struct Sphere
{
    vec4 p_r;
    vec4 col;
};

layout(set = 0, binding = 2, std430) buffer restrict SphereBuffer
{
    Sphere data[];
} s_spheres;

struct Cone
{
    vec4 pa_ra;
    vec4 pb_rb;
    vec4 col;
};

layout(set = 0, binding = 3, std430) buffer restrict ConeBuffer
{
    Cone data[];
} s_cones;

// https://iquilezles.org/articles/intersectors/
float sphere_intersect( in vec3 ro, in vec3 rd, in vec4 sph )
{
    vec3 oc = ro - sph.xyz;
    float b = dot( oc, rd );
    float c = dot( oc, oc ) - sph.w*sph.w;
    float h = b*b - c;
    if( h<0.0 ) return -1.0;
    return -b - sqrt( h );
}

float dot2( in vec3 v ) { return dot(v,v); }

float capped_cone_intersect( in vec3  ro, in vec3 rd, in vec3 pa, in vec3 pb, in float ra, in float rb )
{
    vec3  ba = pb - pa;
    vec3  oa = ro - pa;
    vec3  ob = ro - pb;

    float m0 = dot(ba,ba);
    float m1 = dot(oa,ba);
    float m2 = dot(ob,ba);
    float m3 = dot(rd,ba);

    //caps
    if( m1<0.0 ) { if( dot2(oa*m3-rd*m1)<(ra*ra*m3*m3) ) return -m1/m3; }
    else if( m2>0.0 ) { if( dot2(ob*m3-rd*m2)<(rb*rb*m3*m3) ) return -m2/m3; }

    // body
    float m4 = dot(rd,oa);
    float m5 = dot(oa,oa);
    float rr = ra - rb;
    float hy = m0 + rr*rr;

    float k2 = m0*m0    - m3*m3*hy;
    float k1 = m0*m0*m4 - m1*m3*hy + m0*ra*(rr*m3*1.0        );
    float k0 = m0*m0*m5 - m1*m1*hy + m0*ra*(rr*m1*2.0 - m0*ra);

    float h = k1*k1 - k2*k0;
    if( h<0.0 ) return -1.0;

    float t = (-k1-sqrt(h))/k2;

    float y = m1 + t*m3;
    if( y>0.0 && y<m0 )
    {
        return t;
    }

    return -1.0;
}

void main()
{
    vec3 ndc = vec3((in_uv) * vec2(2.0, 2.0) - vec2(1.0, 1.0), 1.0);
    vec4 target_pos = u_frame.inv_view_proj_matrix * vec4(ndc, 1.0); target_pos = target_pos / target_pos.w;
    vec3 ro = u_frame.view_position.xyz;
    vec3 rd = normalize(target_pos.xyz - ro);

    float t = -1.0;
    float final_t = -1.0;
    vec3 final_col = vec3(0.0);

    for (uint i = 0; i < u_params.geom_count.x; ++i)
    {
        Sphere shp = s_spheres.data[i];
        t = sphere_intersect(ro, rd, shp.p_r);
        if (t > 0 && (final_t < 0.0 || t < final_t))
        {
            final_t = t;
            final_col = shp.col.xyz;
        }
    }

    for (uint i = 0; i < u_params.geom_count.y; ++i)
    {
        Cone cone = s_cones.data[i];
        t = capped_cone_intersect(ro, rd, cone.pa_ra.xyz, cone.pb_rb.xyz, cone.pa_ra.w, cone.pb_rb.w);
        if (t > 0 && (final_t < 0.0 || t < final_t))
        {
            final_t = t;
            final_col = cone.col.xyz;
        }
    }

    if (final_t > 0.0)
    {
        out_color = vec4(final_col, 0.5);
        return;
    }
    out_color = vec4(0.0);
}