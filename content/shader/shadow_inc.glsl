#ifndef SHADOW_INC_H
#define SHADOW_INC_H

#define SHADOW_CASCADE_COUNT 3

struct PerShadowInfo
{
    mat4 light_matrixs[6];
    vec4 cascade_splits;
};

#ifdef USING_SHADOW_INFO
layout(set = 0, binding = USING_SHADOW_INFO) uniform ShadowInfoBuffer
{
    PerShadowInfo data[1];
} u_shadow_infos;
#endif

#ifdef USING_SHADOW_MAP
#if defined(SHADOW_VSM)
layout(set = 0, binding = USING_SHADOW_MAP) uniform sampler2DArray t_shadow_map;

float linstep(const float a, const float b, const float v)
{
    return saturate((v - a) / (b - a));
}

// Reduces VSM light bleedning
float reduce_light_bleed(const float p_max, const float amount)
{
    // Remove the [0, amount] tail and linearly rescale (amount, 1].
    return linstep(amount, 1.0, p_max);
}

float chebyshev_upper_bound(vec2 moments, float mean, float min_variance, float light_bleed_reduction)
{
    // Donnelly and Lauritzen 2006, "Variance Shadow Maps"
    float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, min_variance);

    float d = mean - moments.x;
    float p_max = variance / (variance + d * d);

    p_max = reduce_light_bleed(p_max, light_bleed_reduction);

    return mean <= moments.x ? 1.0 : p_max;
}

float evaluate_vsm(vec2 moments, float depth)
{
    float light_bleeding_reduction = 0.4;
    float min_variance = 0.001 * depth * depth;
    return chebyshev_upper_bound(moments, depth, min_variance, light_bleeding_reduction);
}

float sampler_shadow_vsm(uint cascade, vec3 shadow_position)
{
    vec2 uv = shadow_position.xy;
    vec4 moments = texture(t_shadow_map, vec3(uv, cascade));
    float depth = shadow_position.z;
    depth = depth * 2.0 - 1.0;
    depth = VSM_EXPONENT * depth;
    depth = exp(depth);

    float p = evaluate_vsm(moments.xy, depth);
    p = min(p, evaluate_vsm(moments.zw, -1.0 / depth));
    return p;
}

#else
layout(set = 0, binding = USING_SHADOW_MAP) uniform sampler2DArrayShadow t_shadow_map;

float sampler_shadow(uint cascade, vec3 shadow_position, float bias)
{
    vec2 uv = shadow_position.xy;
    float depth = shadow_position.z - bias;
    return texture(t_shadow_map, vec4(uv, cascade, saturate(depth)));
}

float sampler_shadow_pcf(uint cascade, vec3 shadow_position, float bias)
{
    vec2 size = vec2(textureSize(t_shadow_map, 0));
    vec2 texel_size = vec2(1.0) / size;
    vec2 uv = shadow_position.xy;
    float depth = shadow_position.z - bias;

    float sum = 0.0;
    sum += texture(t_shadow_map, vec4(uv, cascade, saturate(depth)));

    sum += texture(t_shadow_map, vec4(uv + vec2(texel_size.x, 0.0), cascade, saturate(depth)));
    sum += texture(t_shadow_map, vec4(uv + vec2(-texel_size.x, 0.0), cascade, saturate(depth)));

    sum += texture(t_shadow_map, vec4(uv + vec2(0.0, texel_size.y), cascade, saturate(depth)));
    sum += texture(t_shadow_map, vec4(uv + vec2(0.0, -texel_size.y), cascade, saturate(depth)));

    sum += texture(t_shadow_map, vec4(uv + vec2(texel_size.x, texel_size.y), cascade, saturate(depth)));
    sum += texture(t_shadow_map, vec4(uv + vec2(texel_size.x, texel_size.y), cascade, saturate(depth)));

    sum += texture(t_shadow_map, vec4(uv + vec2(-texel_size.x, texel_size.y), cascade, saturate(depth)));
    sum += texture(t_shadow_map, vec4(uv + vec2(-texel_size.x, -texel_size.y), cascade, saturate(depth)));

    return sum * (1.0 / 9.0);
}

float sampler_shadow_pcf_optimized(uint cascade, vec3 shadow_position, float bias)
{
    vec2 size = vec2(textureSize(t_shadow_map, 0));
    vec2 texel_size = vec2(1.0) / size;
    vec2 uv = shadow_position.xy * size;
    float depth = shadow_position.z - bias;

    vec2 base_uv;
    base_uv.x = floor(uv.x + 0.5);
    base_uv.y = floor(uv.y + 0.5);
    float s = (uv.x + 0.5 - base_uv.x);
    float t = (uv.y + 0.5 - base_uv.y);
    base_uv -= vec2(0.5, 0.5);
    base_uv *= texel_size;

    float sum = 0.0;
    float uw0 = (3 - 2 * s);
    float uw1 = (1 + 2 * s);

    float u0 = (2 - s) / uw0 - 1;
    float u1 = s / uw1 + 1;

    float vw0 = (3 - 2 * t);
    float vw1 = (1 + 2 * t);

    float v0 = (2 - t) / vw0 - 1;
    float v1 = t / vw1 + 1;

    u0 *= texel_size.x;
    u1 *= texel_size.x;
    v0 *= texel_size.y;
    v1 *= texel_size.y;

    sum += uw0 * vw0 * texture(t_shadow_map, vec4(base_uv + vec2(u0, v0), cascade, saturate(depth)));
    sum += uw1 * vw0 * texture(t_shadow_map, vec4(base_uv + vec2(u1, v0), cascade, saturate(depth)));
    sum += uw0 * vw1 * texture(t_shadow_map, vec4(base_uv + vec2(u0, v1), cascade, saturate(depth)));
    sum += uw1 * vw1 * texture(t_shadow_map, vec4(base_uv + vec2(u1, v1), cascade, saturate(depth)));

    return sum / 16.0;
}

vec2 poisson_samples[64] = vec2[](
vec2(0.511749, 0.547686), vec2(0.58929, 0.257224), vec2(0.165018, 0.57663), vec2(0.407692, 0.742285),
vec2(0.707012, 0.646523), vec2(0.31463, 0.466825), vec2(0.801257, 0.485186), vec2(0.418136, 0.146517),
vec2(0.579889, 0.0368284), vec2(0.79801, 0.140114), vec2(-0.0413185, 0.371455), vec2(-0.0529108, 0.627352),
vec2(0.0821375, 0.882071), vec2(0.17308, 0.301207), vec2(-0.120452, 0.867216), vec2(0.371096, 0.916454),
vec2(-0.178381, 0.146101), vec2(-0.276489, 0.550525), vec2(0.12542, 0.126643), vec2(-0.296654, 0.286879),
vec2(0.261744, -0.00604975), vec2(-0.213417, 0.715776), vec2(0.425684, -0.153211), vec2(-0.480054, 0.321357),
vec2(-0.0717878, -0.0250567), vec2(-0.328775, -0.169666), vec2(-0.394923, 0.130802), vec2(-0.553681, -0.176777),
vec2(-0.722615, 0.120616), vec2(-0.693065, 0.309017), vec2(0.603193, 0.791471), vec2(-0.0754941, -0.297988),
vec2(0.109303, -0.156472), vec2(0.260605, -0.280111), vec2(0.129731, -0.487954), vec2(-0.537315, 0.520494),
vec2(-0.42758, 0.800607), vec2(0.77309, -0.0728102), vec2(0.908777, 0.328356), vec2(0.985341, 0.0759158),
vec2(0.947536, -0.11837), vec2(-0.103315, -0.610747), vec2(0.337171, -0.584), vec2(0.210919, -0.720055),
vec2(0.41894, -0.36769), vec2(-0.254228, -0.49368), vec2(-0.428562, -0.404037), vec2(-0.831732, -0.189615),
vec2(-0.922642, 0.0888026), vec2(-0.865914, 0.427795), vec2(0.706117, -0.311662), vec2(0.545465, -0.520942),
vec2(-0.695738, 0.664492), vec2(0.389421, -0.899007), vec2(0.48842, -0.708054), vec2(0.760298, -0.62735),
vec2(-0.390788, -0.707388), vec2(-0.591046, -0.686721), vec2(-0.769903, -0.413775), vec2(-0.604457, -0.502571),
vec2(-0.557234, 0.00451362), vec2(0.147572, -0.924353), vec2(-0.0662488, -0.892081), vec2(0.863832, -0.407206)
);

float sampler_shadow_pcf_random_disc(uint cascade, vec3 shadow_position, float bias)
{
    vec2 size = vec2(textureSize(t_shadow_map, 0));
    vec2 texel_size = vec2(1.0) / size;
    vec2 uv = shadow_position.xy;
    float depth = shadow_position.z - bias;
    vec2 sample_scale = 2.0f * texel_size;

    mat2 disk_rotation;
    {
        float r = quick_hash(gl_FragCoord.xy) * 2.0 * PI;
        float sr = sin(r);
        float cr = cos(r);
        disk_rotation = mat2(vec2(cr, -sr), vec2(sr, cr));
    }

    uint num_samples = 8;
    float sum = 0.0;
    for (uint i = 0; i < num_samples; i++)
    {
        vec2 sample_offset = (disk_rotation * poisson_samples[i]) * sample_scale;
        sum += texture(t_shadow_map, vec4(uv + sample_offset, cascade, saturate(depth)));
    }

    return sum * (1.0 / num_samples);
}
#endif

vec2 get_shadow_texel_size()
{
    vec2 size = vec2(textureSize(t_shadow_map, 0));
    vec2 texel_size = vec2(1.0) / size;
    return texel_size;
}

vec2 get_shadow_offsets(vec3 N, vec3 L)
{
    float cos_alpha = saturate(dot(N, L));
    float offset_scale_N = sqrt(1 - cos_alpha * cos_alpha);
    float offset_scale_L = offset_scale_N / cos_alpha;
    return vec2(offset_scale_N, min(2, offset_scale_L));
}

#endif

#endif