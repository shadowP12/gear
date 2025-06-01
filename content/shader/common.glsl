#ifndef COMMON_H
#define COMMON_H

#define PI                 3.14159265359
#define HALF_PI            1.570796327
#define VSM_EXPONENT       5.54 // 42.0(32Bit)

#define saturate(x)        clamp(x, 0.0, 1.0)

float quick_hash(vec2 pos)
{
    const vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
    return fract(magic.z * fract(dot(pos, magic.xy)));
}

#endif