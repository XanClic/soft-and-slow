#include ".soft-and-slow-general.hpp"

vec2::vec2(const vec3 &xyz)
{
    v[0] = xyz.v[0];
    v[1] = xyz.v[1];
}

vec2::vec2(const vec4 &xyzw)
{
    v[0] = xyzw.v[0];
    v[1] = xyzw.v[1];
}

vec3::vec3(const vec4 &xyzw)
{
    v[0] = xyzw.v[0];
    v[1] = xyzw.v[1];
    v[2] = xyzw.v[2];
}

mat4::mat4(const mat3 &m)
{
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            v[x * 4 + y] = m.v[x * 3 + y];

    v[ 3] = 0.f;
    v[ 7] = 0.f;
    v[11] = 0.f;
    v[12] = 0.f;
    v[13] = 0.f;
    v[14] = 0.f;
    v[15] = 1.f;
}
