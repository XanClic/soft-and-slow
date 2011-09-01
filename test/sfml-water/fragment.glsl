#version 110

uniform float offset;
uniform vec3 cam_pos;

uniform sampler2D heightmap;

const float e = 1. / 256.;

varying vec3 vertex_pos;


void main(void)
{
    float u = gl_TexCoord[0].s + offset / 1000.;
    float v = gl_TexCoord[0].t + offset / 500.;
    u = mod(u, 1.);
    v = mod(v, 1.);

    float z1 = texture2D(heightmap, vec2(u, v)).r / 64.;
    float z2 = texture2D(heightmap, vec2(u + e, v)).r / 64.;
    float z3 = texture2D(heightmap, vec2(u, v + e)).r / 64.;

    // Normalenvektor der Heightmap
    vec3 n = normalize(gl_NormalMatrix * (e * vec3(z1 - z2, z1 - z3, e)));

    vec3 sr = reflect(vec3(0., 0., -1.), n);


    vec3 cf = normalize(vertex_pos - cam_pos);

    float sb = dot(sr, cf);


    vec4 water_color = vec4(0., 0., 0., 0.);
    vec4 sun_reflection = vec4(0., 0., 0., 0.);

    if (sb <= -.99)
        sun_reflection = vec4(1., 1., 1., 1.);

    if (sun_reflection.a < 1.)
    {
        float b = (3.2 - 2. * sb) * .25;

        water_color = vec4(.1 * b, .5 * b, b, 1.1 - dot(cf, vec3(0., -1., 0.)) * .5);
    }

    if (sun_reflection.a == 1.)
        water_color = sun_reflection;
    else if (sun_reflection.a > 0.)
        water_color = vec4(mix(water_color.rgb, sun_reflection.rgb, sun_reflection.a), max(water_color.a, sun_reflection.a));


    gl_FragColor = water_color;
}
