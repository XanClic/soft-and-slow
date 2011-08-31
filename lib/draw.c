#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/context.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/helpers.h>
#include <soft-and-slow/pipeline.h>
#include <soft-and-slow/shader.h>
#include <soft-and-slow/types.h>


extern sas_context_t current_sas_context;

extern GLenum sas_error;
extern GLenum sas_current_mode;

extern sas_color_t sas_current_color;
extern float sas_multi_texcoord0[4], sas_current_vertex[4];

extern float sas_current_position[4], sas_triangle_positions[3][4];
extern float sas_triangle_texcoords[3][4], sas_current_texcoord[8][4];
extern sas_color_t sas_triangle_colors[3];

extern float sas_quad_positions[4][4], sas_quad_texcoords[4][4];
extern sas_color_t sas_quad_colors[4];

extern int sas_triangle_index, sas_quad_index;

extern bool sas_do_cw_culling, sas_do_ccw_culling;

extern unsigned sas_current_buf_index;


// Current value for the check buffer
static uint8_t primitive_counter = 1;


void glBegin(GLenum mode)
{
    switch (mode)
    {
        case GL_POINTS:
            sas_current_mode = GL_POINTS;
            sas_varyings_alloc(1);
            break;
        case GL_TRIANGLES:
            sas_triangle_index = 0;
            sas_current_mode = GL_TRIANGLES;
            sas_varyings_alloc(3);
            break;
        case GL_QUADS:
            sas_quad_index = 0;
            sas_current_mode = GL_QUADS;
            sas_varyings_alloc(4);
            break;
        default:
            sas_error = GL_INVALID_ENUM;
    }
}

void glEnd(void)
{
    sas_current_mode = -1;
}


void glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
    sas_current_color.r = r;
    sas_current_color.g = g;
    sas_current_color.b = b;
    sas_current_color.a = 1.f;
}

void glTexCoord2f(GLfloat s, GLfloat v)
{
    sas_multi_texcoord0[0] = s;
    sas_multi_texcoord0[1] = v;
    sas_multi_texcoord0[2] = 0.f;
    sas_multi_texcoord0[3] = 1.f;
}


static float pos_multiplier[4] = { .5f, .5f, .5f, 1.f };
static float pos_addend[4]     = { .5f, .5f, .5f, 0.f };
static float xmm_one[4]        = { 1.f, 1.f, 1.f, 1.f };


// Fills a triangle.
void sas_do_triangle(sas_color_t c1, float *t1, float *v1, int i1, sas_color_t c2, float *t2, float *v2, int i2, sas_color_t c3, float *t3, float *v3, int i3)
{
#ifdef USE_ASSEMBLY
    float vec1[4], vec2[4];

    __asm__ __volatile__ ("movaps xmm0,[%2];"
                          "movaps xmm1,[%3];"
                          "movaps xmm2,[%4];"
                          "subps  xmm1,xmm0;"
                          "subps  xmm2,xmm0;"
                          "movaps %0  ,xmm1;"
                          "movaps %1  ,xmm2;"
                          : "=m"(vec1), "=m"(vec2)
                          : "r"(v1), "r"(v2), "r"(v3)
                          : "xmm0", "xmm1", "xmm2");
#else
    float vec1[4] = {
        v2[0] - v1[0],
        v2[1] - v1[1],
        v2[2] - v1[2],
        v2[3] - v1[3]
    };

    float vec2[4] = {
        v3[0] - v1[0],
        v3[1] - v1[1],
        v3[2] - v1[2],
        v3[3] - v1[3]
    };
#endif

    if (sas_do_cw_culling || sas_do_ccw_culling)
    {
        // Some sort of fast cross product, looking for the relevant part
        // (depth coordinate of the perpendicular vector)
        if (vec1[0] * vec2[1] > vec1[1] * vec2[0]) // true for CCW
        {
            if (sas_do_ccw_culling)
                return; // cull that
        }
        else if (sas_do_cw_culling)
            return; // cull that
    }


    // Vertices' depth values (in range [-1; 1], bring to [0; 1]); subtract from
    // 1 (Wikipedia tells us to use the reciprocal instead but that doesn't seem
    // to work).
    float d1 = .5f - v1[2] / 2.f;
    float d2 = .5f - v2[2] / 2.f;
    float d3 = .5f - v3[2] / 2.f;


    // This isn't actually true as it draws too many pixels -- but it works.
    float unit1 = 1.f / sqrtf(vec1[0] * vec1[0] * current_sas_context->width * current_sas_context->width + vec1[1] * vec1[1] * current_sas_context->height * current_sas_context->height);
    float unit2 = 1.f / sqrtf(vec2[0] * vec2[0] * current_sas_context->width * current_sas_context->width + vec2[1] * vec2[1] * current_sas_context->height * current_sas_context->height);


    bool s_set_to_one = false;

    for (float s = 0.f; s <= 1.f; s += unit1)
    {
        bool t_set_to_one = false;

        for (float t = 0.f; t <= 1.f - s; t += unit2)
        {
#ifdef USE_ASSEMBLY
            __asm__ __volatile__ ("pshufd   xmm8,%1,0x00;"
                                  "mulps    xmm8,[%4];"
                                  "pshufd   xmm9,%2,0x00;"
                                  "mulps    xmm9,[%5];"
                                  "addps    xmm8,xmm9;"
                                  "addps    xmm8,[%3];"
                                  "mulps    xmm8,[%6];"
                                  "addps    xmm8,[%7];"
                                  "movaps   %0,xmm8"
                                  : "=m"(sas_current_position)
                                  : "x"(s), "x"(t), "r"(v1), "r"(vec1), "r"(vec2), "r"(pos_multiplier), "r"(pos_addend)
                                  : "xmm8", "xmm9");
#else
            sas_current_position[0] = v1[0] + s * vec1[0] + t * vec2[0];
            sas_current_position[1] = v1[1] + s * vec1[1] + t * vec2[1];
            sas_current_position[2] = v1[2] + s * vec1[2] + t * vec2[2];
            sas_current_position[3] = v1[3] + s * vec1[3] + t * vec2[3];

            // Bring to [0; 1]
            sas_current_position[0] = sas_current_position[0] * .5f + .5f;
            sas_current_position[1] = sas_current_position[1] * .5f + .5f;
            sas_current_position[2] = sas_current_position[2] * .5f + .5f;
#endif

            if ((sas_current_position[0] < 0.f) || (sas_current_position[0] >= 1.f) ||
                (sas_current_position[1] < 0.f) || (sas_current_position[1] >= 1.f) ||
                (sas_current_position[2] < 0.f) || (sas_current_position[2] >= 1.f))
                goto cull_fragment;

            sas_current_buf_index = (current_sas_context->height - 1 - (unsigned)(sas_current_position[1] * current_sas_context->height)) * current_sas_context->width + (unsigned)(sas_current_position[0] * current_sas_context->width);

            if (current_sas_context->__checkbuffer[sas_current_buf_index] == primitive_counter)
                goto cull_fragment;

            current_sas_context->__checkbuffer[sas_current_buf_index] = primitive_counter;


#ifdef USE_ASSEMBLY
            float w1, w2, w3, dd;

            __asm__ __volatile__ ("pshufd   xmm13,%6,0x00;" // w2 = s
                                  "pshufd   xmm14,%7,0x00;" // w3 = t
                                  "movaps   xmm12,[%8];"
                                  "subps    xmm12,xmm13;"
                                  "subps    xmm12,xmm14;"   // w1 = 1 - s - t
                                  "pshufd   xmm0,%9,0x00;"
                                  "mulps    xmm12,xmm0;"    // w1 *= d1
                                  "pshufd   xmm0,%10,0x00;"
                                  "mulps    xmm13,xmm0;"    // w2 *= d2
                                  "pshufd   xmm0,%11,0x00;"
                                  "mulps    xmm14,xmm0;"    // w3 *= d3
                                  "movaps   xmm15,xmm12;"
                                  "addps    xmm15,xmm13;"
                                  "addps    xmm15,xmm14;"
                                  "rcpps    xmm15,xmm15;"   // dd = 1 / (w1 + w2 + w3)
                                  "movss    %0,xmm12;"
                                  "movss    %1,xmm13;"
                                  "movss    %2,xmm14;"
                                  "movss    %3,xmm15;"

                                  "movaps   xmm0,%12;"
                                  "mulps    xmm0,xmm12;"
                                  "movaps   xmm1,%13;"
                                  "mulps    xmm1,xmm13;"
                                  "addps    xmm0,xmm1;"
                                  "movaps   xmm1,%14;"
                                  "mulps    xmm1,xmm14;"
                                  "addps    xmm0,xmm1;"
                                  "mulps    xmm0,xmm15;"
                                  "movaps   %4,xmm0;"

                                  "mulps    xmm12,%15;"
                                  "mulps    xmm13,%16;"
                                  "mulps    xmm14,%17;"
                                  "addps    xmm12,xmm13;"
                                  "addps    xmm12,xmm14;"
                                  "mulps    xmm12,xmm15;"
                                  "movaps   %5,xmm12"
                                  : "=x"(w1), "=x"(w2), "=x"(w3), "=x"(dd), "=m"(sas_current_color), "=m"(sas_current_texcoord[0])
                                  : "x"(s), "x"(t), "r"(xmm_one), "x"(d1), "x"(d2), "x"(d3), "m"(c1), "m"(c2), "m"(c3), "m"(*(sas_color_t *)t1), "m"(*(sas_color_t *)t2), "m"(*(sas_color_t *)t3)
                                  : "xmm0", "xmm1", "xmm12", "xmm13", "xmm14", "xmm15");
#else
            // Get weighting
            float w1 = 1.f - s - t, w2 = s, w3 = t;

            // Multiply by inverse depth in order to map perspectively correct
            w1 *= d1;
            w2 *= d2;
            w3 *= d3;

            // Depth multiplier for perspective correct mapping
            float dd = 1.f / (w1 + w2 + w3);

            sas_current_color.r = (c1.r * w1 + c2.r * w2 + c3.r * w3) * dd;
            sas_current_color.g = (c1.g * w1 + c2.g * w2 + c3.g * w3) * dd;
            sas_current_color.b = (c1.b * w1 + c2.b * w2 + c3.b * w3) * dd;
            sas_current_color.a = (c1.a * w1 + c2.a * w2 + c3.a * w3) * dd;

            // TODO: Which texture unit to use
            sas_current_texcoord[0][0] = (t1[0] * w1 + t2[0] * w2 + t3[0] * w3) * dd;
            sas_current_texcoord[0][1] = (t1[1] * w1 + t2[1] * w2 + t3[1] * w3) * dd;
            sas_current_texcoord[0][2] = (t1[2] * w1 + t2[2] * w2 + t3[2] * w3) * dd;
            sas_current_texcoord[0][3] = (t1[3] * w1 + t2[3] * w2 + t3[3] * w3) * dd;
#endif

            sas_calc_varyings(i1, i2, i3, w1, w2, w3, dd);

            sas_transform_fragment();


cull_fragment:
            if ((t + unit2 > 1.f - s) && !t_set_to_one)
            {
                t = 1.f - s - unit2;
                t_set_to_one = true;
            }
        }


        if ((s + unit1 > 1.f) && !s_set_to_one)
        {
            s = 1.f - unit1;
            s_set_to_one = true;
        }
    }
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    sas_current_vertex[0] = x;
    sas_current_vertex[1] = y;
    sas_current_vertex[2] = z;
    sas_current_vertex[3] = 1.f;

    sas_transform_vertex_to_screen();

    sas_push_varyings();

    if (sas_current_mode == GL_POINTS)
    {
        sas_transform_fragment();

        sas_flush_varyings();
    }
    else if (sas_current_mode == GL_TRIANGLES)
    {
        sas_triangle_colors[sas_triangle_index] = sas_current_color;
        memcpy(sas_triangle_texcoords[sas_triangle_index], sas_multi_texcoord0, sizeof(sas_multi_texcoord0));
        memcpy(sas_triangle_positions[sas_triangle_index++], sas_current_position, sizeof(sas_current_position));

        if (sas_triangle_index == 3)
        {
            sas_triangle_index = 0;

            // On overflow: clear the buffer so we won't get pixels which seem
            // to be already drawn but aren't.
            if (!++primitive_counter)
            {
                memset(current_sas_context->__checkbuffer, 0, current_sas_context->width * current_sas_context->height);
                // Set the counter to one, because all pixels in the buffer
                // are now 0.
                primitive_counter = 1;
            }

            sas_do_triangle(sas_triangle_colors[0], sas_triangle_texcoords[0], sas_triangle_positions[0], 0,
                            sas_triangle_colors[1], sas_triangle_texcoords[1], sas_triangle_positions[1], 1,
                            sas_triangle_colors[2], sas_triangle_texcoords[2], sas_triangle_positions[2], 2);


            sas_flush_varyings();
        }
    }
    else if (sas_current_mode == GL_QUADS)
    {
        sas_quad_colors[sas_quad_index] = sas_current_color;
        memcpy(sas_quad_texcoords[sas_quad_index], sas_multi_texcoord0, sizeof(sas_multi_texcoord0));
        memcpy(sas_quad_positions[sas_quad_index++], sas_current_position, sizeof(sas_current_position));


        if (sas_quad_index == 4)
        {
            sas_quad_index = 0;

            if (!++primitive_counter)
            {
                memset(current_sas_context->__checkbuffer, 0, current_sas_context->width * current_sas_context->height);
                primitive_counter = 1;
            }

            sas_do_triangle(sas_quad_colors[0], sas_quad_texcoords[0], sas_quad_positions[0], 0,
                            sas_quad_colors[1], sas_quad_texcoords[1], sas_quad_positions[1], 1,
                            sas_quad_colors[2], sas_quad_texcoords[2], sas_quad_positions[2], 2);

            sas_do_triangle(sas_quad_colors[2], sas_quad_texcoords[2], sas_quad_positions[2], 2,
                            sas_quad_colors[3], sas_quad_texcoords[3], sas_quad_positions[3], 3,
                            sas_quad_colors[0], sas_quad_texcoords[0], sas_quad_positions[0], 0);


            sas_flush_varyings();
        }
    }
}
