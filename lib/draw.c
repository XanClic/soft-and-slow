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
#include <soft-and-slow/threads.h>
#include <soft-and-slow/types.h>


extern sas_context_t current_sas_context;

extern GLenum sas_error;
extern GLenum sas_current_mode;

extern sas_color_t sas_current_color;
extern float sas_multi_texcoord0[4], sas_current_vertex[4];
extern float sas_current_normal[3];

extern float sas_current_position[4], sas_triangle_positions[3][4];
extern float sas_triangle_texcoords[3][4], sas_current_texcoord[8][4];
extern sas_color_t sas_triangle_colors[3];

extern float sas_quad_positions[4][4], sas_quad_texcoords[4][4];
extern sas_color_t sas_quad_colors[4];

extern int sas_triangle_index, sas_quad_index;

extern bool sas_do_cw_culling, sas_do_ccw_culling;
extern bool sas_normalize_normals;
extern bool sas_smooth_shading;

extern unsigned sas_current_buf_index;


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
        case GL_QUAD_STRIP:
            sas_quad_index = 0;
            sas_current_mode = GL_QUAD_STRIP;
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

void glNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
    if (!sas_normalize_normals)
    {
        sas_current_normal[0] = x;
        sas_current_normal[1] = y;
        sas_current_normal[2] = z;
    }
    else
    {
        float fac = 1.f / sqrtf(x * x + y * y + z * z);

        sas_current_normal[0] = x * fac;
        sas_current_normal[1] = y * fac;
        sas_current_normal[2] = z * fac;
    }
}


static float xmm_one[4]        = { 1.f, 1.f, 1.f, 1.f };


#ifdef THREADING
void sas_do_triangle(void *dti_voidptr)
{
    sas_draw_thread_info_t *dti = dti_voidptr;

    float vec1[4] = {
        (dti->vertex_positions[1][0] - dti->vertex_positions[0][0]) * .5f,
        (dti->vertex_positions[1][1] - dti->vertex_positions[0][1]) * .5f,
        (dti->vertex_positions[1][2] - dti->vertex_positions[0][2]) * .5f,
         dti->vertex_positions[1][3] - dti->vertex_positions[0][3]
    };

    float vec2[4] = {
        (dti->vertex_positions[2][0] - dti->vertex_positions[0][0]) * .5f,
        (dti->vertex_positions[2][1] - dti->vertex_positions[0][1]) * .5f,
        (dti->vertex_positions[2][2] - dti->vertex_positions[0][2]) * .5f,
         dti->vertex_positions[2][3] - dti->vertex_positions[0][3]
    };

    float bv[4] = {
        dti->vertex_positions[0][0] * .5f + .5f,
        dti->vertex_positions[0][1] * .5f + .5f,
        dti->vertex_positions[0][2] * .5f + .5f,
        dti->vertex_positions[0][3]
    };

    if (sas_do_cw_culling || sas_do_ccw_culling)
    {
        if (vec1[0] * vec2[1] > vec1[1] * vec2[0])
        {
            if (sas_do_ccw_culling)
                return;
        }
        else if (sas_do_cw_culling)
            return;
    }


    float d1 = .5f - dti->vertex_positions[0][2] * .5f;
    float d2 = .5f - dti->vertex_positions[1][2] * .5f;
    float d3 = .5f - dti->vertex_positions[2][2] * .5f;


    float st_div = 1.f / (vec1[0] * vec2[1] - vec1[1] * vec2[0]);


    int start_x = (int)floorf((min(dti->vertex_positions[0][0], min(dti->vertex_positions[1][0], dti->vertex_positions[2][0])) * .5f + .5f) * current_sas_context->width );
    int end_x   = (int)ceilf ((max(dti->vertex_positions[0][0], max(dti->vertex_positions[1][0], dti->vertex_positions[2][0])) * .5f + .5f) * current_sas_context->width );
    int start_y = (int)floorf((.5f - max(dti->vertex_positions[0][1], max(dti->vertex_positions[1][1], dti->vertex_positions[2][1])) * .5f) * current_sas_context->height);
    int end_y   = (int)ceilf ((.5f - min(dti->vertex_positions[0][1], min(dti->vertex_positions[1][1], dti->vertex_positions[2][1])) * .5f) * current_sas_context->height);

    if (start_x < 0)
        start_x = 0;
    if (start_y < 0)
        start_y = 0;

    if (end_x >= (int)current_sas_context->width)
        end_x = current_sas_context->width - 1;
    if (end_y >= (int)current_sas_context->height)
        end_y = current_sas_context->height - 1;


    for (int y = start_y; y < end_y; y++)
    {
        dti->current_buf_index = y * current_sas_context->width + start_x;

        for (int x = start_x; x < end_x; x++)
        {
            dti->current_position[0] =       (float)x / (float)current_sas_context->width;
            dti->current_position[1] = 1.f - (float)y / (float)current_sas_context->height;

            dti->current_buf_index++;

            float xs = dti->current_position[0] - bv[0];
            float ys = dti->current_position[1] - bv[1];

            float s =  (xs * vec2[1] - ys * vec2[0]) * st_div;
            float t = -(xs * vec1[1] - ys * vec1[0]) * st_div;

            if ((s < 0.f) || (t < 0.f) || (s + t > 1.f))
                continue;

            dti->current_position[2] = bv[2] + s * vec1[2] + t * vec2[2];
            dti->current_position[3] = bv[3] + s * vec1[3] + t * vec2[3];


            if ((dti->current_position[2] < 0.f) || (dti->current_position[2] >= 1.f))
                continue;


#ifdef USE_ASSEMBLY
            float w1, w2, w3, dd;

            if (sas_smooth_shading)
            {
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
                                      : "=x"(w1), "=x"(w2), "=x"(w3), "=x"(dd), "=m"(dti->current_color), "=m"(dti->current_texcoord[0])
                                      : "x"(s), "x"(t), "r"(xmm_one),
                                        "x"(d1), "x"(d2), "x"(d3),
                                        "m"(dti->vertex_colors[0]), "m"(dti->vertex_colors[1]), "m"(dti->vertex_colors[2]),
                                        "m"(*(sas_xmm_t *)dti->vertex_texcoords[0]), "m"(*(sas_xmm_t *)dti->vertex_texcoords[1]), "m"(*(sas_xmm_t *)dti->vertex_texcoords[2])
                                      : "xmm0", "xmm1", "xmm12", "xmm13", "xmm14", "xmm15");
            }
            else
            {
                __asm__ __volatile__ ("pshufd   xmm13,%5,0x00;" // w2 = s
                                      "pshufd   xmm14,%6,0x00;" // w3 = t
                                      "movaps   xmm12,[%7];"
                                      "subps    xmm12,xmm13;"
                                      "subps    xmm12,xmm14;"   // w1 = 1 - s - t
                                      "pshufd   xmm0,%8,0x00;"
                                      "mulps    xmm12,xmm0;"    // w1 *= d1
                                      "pshufd   xmm0,%9,0x00;"
                                      "mulps    xmm13,xmm0;"    // w2 *= d2
                                      "pshufd   xmm0,%10,0x00;"
                                      "mulps    xmm14,xmm0;"    // w3 *= d3
                                      "movaps   xmm15,xmm12;"
                                      "addps    xmm15,xmm13;"
                                      "addps    xmm15,xmm14;"
                                      "rcpps    xmm15,xmm15;"   // dd = 1 / (w1 + w2 + w3)
                                      "movss    %0,xmm12;"
                                      "movss    %1,xmm13;"
                                      "movss    %2,xmm14;"
                                      "movss    %3,xmm15;"

                                      "mulps    xmm12,%11;"
                                      "mulps    xmm13,%12;"
                                      "mulps    xmm14,%13;"
                                      "addps    xmm12,xmm13;"
                                      "addps    xmm12,xmm14;"
                                      "mulps    xmm12,xmm15;"
                                      "movaps   %4,xmm12"
                                      : "=x"(w1), "=x"(w2), "=x"(w3), "=x"(dd), "=m"(dti->current_texcoord[0])
                                      : "x"(s), "x"(t), "r"(xmm_one), "x"(d1), "x"(d2), "x"(d3),
                                        "m"(*(sas_xmm_t *)dti->vertex_texcoords[0]), "m"(*(sas_xmm_t *)dti->vertex_texcoords[1]), "m"(*(sas_xmm_t *)dti->vertex_texcoords[2])
                                      : "xmm0", "xmm12", "xmm13", "xmm14", "xmm15");
            }
#else
            // Get weighting
            float w1 = 1.f - s - t, w2 = s, w3 = t;

            // Multiply by inverse depth in order to map perspectively correct
            w1 *= d1;
            w2 *= d2;
            w3 *= d3;

            // Depth multiplier for perspective correct mapping
            float dd = 1.f / (w1 + w2 + w3);

            if (sas_smooth_shading)
            {
                dti->current_color.r = (dti->vertex_colors[0].r * w1 + dti->vertex_colors[1].r * w2 + dti->vertex_colors[2].r * w3) * dd;
                dti->current_color.g = (dti->vertex_colors[0].g * w1 + dti->vertex_colors[1].g * w2 + dti->vertex_colors[2].g * w3) * dd;
                dti->current_color.b = (dti->vertex_colors[0].b * w1 + dti->vertex_colors[1].b * w2 + dti->vertex_colors[2].b * w3) * dd;
                dti->current_color.a = (dti->vertex_colors[0].a * w1 + dti->vertex_colors[1].a * w2 + dti->vertex_colors[2].a * w3) * dd;
            }

            // TODO: Which texture unit to use
            dti->current_texcoord[0][0] = (dti->vertex_texcoords[0][0] * w1 + dti->vertex_texcoords[1][0] * w2 + dti->vertex_texcoords[2][0] * w3) * dd;
            dti->current_texcoord[0][1] = (dti->vertex_texcoords[0][1] * w1 + dti->vertex_texcoords[1][1] * w2 + dti->vertex_texcoords[2][1] * w3) * dd;
            dti->current_texcoord[0][2] = (dti->vertex_texcoords[0][2] * w1 + dti->vertex_texcoords[1][2] * w2 + dti->vertex_texcoords[2][2] * w3) * dd;
            dti->current_texcoord[0][3] = (dti->vertex_texcoords[0][3] * w1 + dti->vertex_texcoords[1][3] * w2 + dti->vertex_texcoords[2][3] * w3) * dd;
#endif

//          sas_calc_varyings(i1, i2, i3, w1, w2, w3, dd);

            sas_transform_fragment(dti);
        }
    }
}
#else
// Fills a triangle.
void sas_do_triangle(sas_color_t c1, float *t1, float *v1, int i1, sas_color_t c2, float *t2, float *v2, int i2, sas_color_t c3, float *t3, float *v3, int i3)
{
    float vec1[4] = {
        (v2[0] - v1[0]) * .5f,
        (v2[1] - v1[1]) * .5f,
        (v2[2] - v1[2]) * .5f,
         v2[3] - v1[3]
    };

    float vec2[4] = {
        (v3[0] - v1[0]) * .5f,
        (v3[1] - v1[1]) * .5f,
        (v3[2] - v1[2]) * .5f,
         v3[3] - v1[3]
    };

    float bv[4] = {
        v1[0] * .5f + .5f,
        v1[1] * .5f + .5f,
        v1[2] * .5f + .5f,
        v1[3]
    };

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


    float st_div = 1.f / (vec1[0] * vec2[1] - vec1[1] * vec2[0]);


    int start_x = (int)floorf((min(v1[0], min(v2[0], v3[0])) * .5f + .5f) * current_sas_context->width );
    int end_x   = (int)ceilf ((max(v1[0], max(v2[0], v3[0])) * .5f + .5f) * current_sas_context->width );
    int start_y = (int)floorf((.5f - max(v1[1], max(v2[1], v3[1])) * .5f) * current_sas_context->height);
    int end_y   = (int)ceilf ((.5f - min(v1[1], min(v2[1], v3[1])) * .5f) * current_sas_context->height);

    if (start_x < 0)
        start_x = 0;
    if (start_y < 0)
        start_y = 0;

    if (end_x >= (int)current_sas_context->width)
        end_x = current_sas_context->width - 1;
    if (end_y >= (int)current_sas_context->height)
        end_y = current_sas_context->height - 1;


    for (int y = start_y; y < end_y; y++)
    {
        sas_current_buf_index = y * current_sas_context->width + start_x;

        for (int x = start_x; x < end_x; x++)
        {
            sas_current_position[0] =       (float)x / (float)current_sas_context->width;
            sas_current_position[1] = 1.f - (float)y / (float)current_sas_context->height;

            sas_current_buf_index++;

            float xs = sas_current_position[0] - bv[0];
            float ys = sas_current_position[1] - bv[1];

            float s =  (xs * vec2[1] - ys * vec2[0]) * st_div;
            float t = -(xs * vec1[1] - ys * vec1[0]) * st_div;

            if ((s < 0.f) || (t < 0.f) || (s + t > 1.f))
                continue;

            sas_current_position[2] = bv[2] + s * vec1[2] + t * vec2[2];
            sas_current_position[3] = bv[3] + s * vec1[3] + t * vec2[3];


            if ((sas_current_position[2] < 0.f) || (sas_current_position[2] >= 1.f))
                continue;


#ifdef USE_ASSEMBLY
            float w1, w2, w3, dd;

            if (sas_smooth_shading)
            {
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
                                      : "x"(s), "x"(t), "r"(xmm_one), "x"(d1), "x"(d2), "x"(d3), "m"(c1), "m"(c2), "m"(c3), "m"(*(sas_xmm_t *)t1), "m"(*(sas_xmm_t *)t2), "m"(*(sas_xmm_t *)t3)
                                      : "xmm0", "xmm1", "xmm12", "xmm13", "xmm14", "xmm15");
            }
            else
            {
                __asm__ __volatile__ ("pshufd   xmm13,%5,0x00;" // w2 = s
                                      "pshufd   xmm14,%6,0x00;" // w3 = t
                                      "movaps   xmm12,[%7];"
                                      "subps    xmm12,xmm13;"
                                      "subps    xmm12,xmm14;"   // w1 = 1 - s - t
                                      "pshufd   xmm0,%8,0x00;"
                                      "mulps    xmm12,xmm0;"    // w1 *= d1
                                      "pshufd   xmm0,%9,0x00;"
                                      "mulps    xmm13,xmm0;"    // w2 *= d2
                                      "pshufd   xmm0,%10,0x00;"
                                      "mulps    xmm14,xmm0;"    // w3 *= d3
                                      "movaps   xmm15,xmm12;"
                                      "addps    xmm15,xmm13;"
                                      "addps    xmm15,xmm14;"
                                      "rcpps    xmm15,xmm15;"   // dd = 1 / (w1 + w2 + w3)
                                      "movss    %0,xmm12;"
                                      "movss    %1,xmm13;"
                                      "movss    %2,xmm14;"
                                      "movss    %3,xmm15;"

                                      "mulps    xmm12,%11;"
                                      "mulps    xmm13,%12;"
                                      "mulps    xmm14,%13;"
                                      "addps    xmm12,xmm13;"
                                      "addps    xmm12,xmm14;"
                                      "mulps    xmm12,xmm15;"
                                      "movaps   %4,xmm12"
                                      : "=x"(w1), "=x"(w2), "=x"(w3), "=x"(dd), "=m"(sas_current_texcoord[0])
                                      : "x"(s), "x"(t), "r"(xmm_one), "x"(d1), "x"(d2), "x"(d3), "m"(*(sas_xmm_t *)t1), "m"(*(sas_xmm_t *)t2), "m"(*(sas_xmm_t *)t3)
                                      : "xmm0", "xmm12", "xmm13", "xmm14", "xmm15");
            }
#else
            // Get weighting
            float w1 = 1.f - s - t, w2 = s, w3 = t;

            // Multiply by inverse depth in order to map perspectively correct
            w1 *= d1;
            w2 *= d2;
            w3 *= d3;

            // Depth multiplier for perspective correct mapping
            float dd = 1.f / (w1 + w2 + w3);

            if (sas_smooth_shading)
            {
                sas_current_color.r = (c1.r * w1 + c2.r * w2 + c3.r * w3) * dd;
                sas_current_color.g = (c1.g * w1 + c2.g * w2 + c3.g * w3) * dd;
                sas_current_color.b = (c1.b * w1 + c2.b * w2 + c3.b * w3) * dd;
                sas_current_color.a = (c1.a * w1 + c2.a * w2 + c3.a * w3) * dd;
            }

            // TODO: Which texture unit to use
            sas_current_texcoord[0][0] = (t1[0] * w1 + t2[0] * w2 + t3[0] * w3) * dd;
            sas_current_texcoord[0][1] = (t1[1] * w1 + t2[1] * w2 + t3[1] * w3) * dd;
            sas_current_texcoord[0][2] = (t1[2] * w1 + t2[2] * w2 + t3[2] * w3) * dd;
            sas_current_texcoord[0][3] = (t1[3] * w1 + t2[3] * w2 + t3[3] * w3) * dd;
#endif

            sas_calc_varyings(i1, i2, i3, w1, w2, w3, dd);

            sas_transform_fragment();
        }
    }
}
#endif

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
#ifndef THREADING
        sas_transform_fragment();
#endif

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

#ifdef THREADING
            sas_draw_thread_info_t dti;

            dti.current_color = sas_current_color;


            memcpy(&dti.vertex_positions[0], &sas_quad_positions[0], sizeof(float) * 4);
            memcpy(&dti.vertex_positions[1], &sas_quad_positions[1], sizeof(float) * 4);
            memcpy(&dti.vertex_positions[2], &sas_quad_positions[2], sizeof(float) * 4);

            dti.vertex_colors[0] = sas_quad_colors[0];
            dti.vertex_colors[1] = sas_quad_colors[1];
            dti.vertex_colors[2] = sas_quad_colors[2];

            memcpy(&dti.vertex_texcoords[0], &sas_quad_texcoords[0], sizeof(float) * 4);
            memcpy(&dti.vertex_texcoords[1], &sas_quad_texcoords[1], sizeof(float) * 4);
            memcpy(&dti.vertex_texcoords[2], &sas_quad_texcoords[2], sizeof(float) * 4);

            dti.vertex_indizes[0] = 0;
            dti.vertex_indizes[1] = 1;
            dti.vertex_indizes[2] = 2;

            sas_thread_execute(sas_do_triangle, &dti, sizeof(dti));
#else
            sas_color_t curcol = sas_current_color;


            sas_do_triangle(sas_triangle_colors[0], sas_triangle_texcoords[0], sas_triangle_positions[0], 0,
                            sas_triangle_colors[1], sas_triangle_texcoords[1], sas_triangle_positions[1], 1,
                            sas_triangle_colors[2], sas_triangle_texcoords[2], sas_triangle_positions[2], 2);


            sas_current_color = curcol;
#endif


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

#ifdef THREADING
            sas_draw_thread_info_t dti;

            dti.current_color = sas_current_color;


            memcpy(&dti.vertex_positions[0], &sas_quad_positions[0], sizeof(float) * 4);
            memcpy(&dti.vertex_positions[1], &sas_quad_positions[1], sizeof(float) * 4);
            memcpy(&dti.vertex_positions[2], &sas_quad_positions[2], sizeof(float) * 4);

            dti.vertex_colors[0] = sas_quad_colors[0];
            dti.vertex_colors[1] = sas_quad_colors[1];
            dti.vertex_colors[2] = sas_quad_colors[2];

            memcpy(&dti.vertex_texcoords[0], &sas_quad_texcoords[0], sizeof(float) * 4);
            memcpy(&dti.vertex_texcoords[1], &sas_quad_texcoords[1], sizeof(float) * 4);
            memcpy(&dti.vertex_texcoords[2], &sas_quad_texcoords[2], sizeof(float) * 4);

            dti.vertex_indizes[0] = 0;
            dti.vertex_indizes[1] = 1;
            dti.vertex_indizes[2] = 2;

            sas_thread_execute(sas_do_triangle, &dti, sizeof(dti));


            memcpy(&dti.vertex_positions[0], &sas_quad_positions[2], sizeof(float) * 4);
            memcpy(&dti.vertex_positions[1], &sas_quad_positions[3], sizeof(float) * 4);
            memcpy(&dti.vertex_positions[2], &sas_quad_positions[0], sizeof(float) * 4);

            dti.vertex_colors[0] = sas_quad_colors[2];
            dti.vertex_colors[1] = sas_quad_colors[3];
            dti.vertex_colors[2] = sas_quad_colors[0];

            memcpy(&dti.vertex_texcoords[0], &sas_quad_texcoords[2], sizeof(float) * 4);
            memcpy(&dti.vertex_texcoords[1], &sas_quad_texcoords[3], sizeof(float) * 4);
            memcpy(&dti.vertex_texcoords[2], &sas_quad_texcoords[0], sizeof(float) * 4);

            dti.vertex_indizes[0] = 2;
            dti.vertex_indizes[1] = 3;
            dti.vertex_indizes[2] = 0;

            sas_thread_execute(sas_do_triangle, &dti, sizeof(dti));
#else
            sas_color_t curcol = sas_current_color;


            sas_do_triangle(sas_quad_colors[0], sas_quad_texcoords[0], sas_quad_positions[0], 0,
                            sas_quad_colors[1], sas_quad_texcoords[1], sas_quad_positions[1], 1,
                            sas_quad_colors[2], sas_quad_texcoords[2], sas_quad_positions[2], 2);

            sas_do_triangle(sas_quad_colors[2], sas_quad_texcoords[2], sas_quad_positions[2], 2,
                            sas_quad_colors[3], sas_quad_texcoords[3], sas_quad_positions[3], 3,
                            sas_quad_colors[0], sas_quad_texcoords[0], sas_quad_positions[0], 0);


            sas_current_color = curcol;
#endif


            sas_flush_varyings();
        }
    }
    else if (sas_current_mode == GL_QUAD_STRIP)
    {
        sas_quad_colors[sas_quad_index] = sas_current_color;
        memcpy(sas_quad_texcoords[sas_quad_index], sas_multi_texcoord0, sizeof(sas_multi_texcoord0));
        memcpy(sas_quad_positions[sas_quad_index++], sas_current_position, sizeof(sas_current_position));


        if (sas_quad_index == 4)
        {
            sas_quad_index = 2;

#ifdef THREADING
            sas_draw_thread_info_t dti;

            dti.current_color = sas_current_color;


            memcpy(&dti.vertex_positions[0], &sas_quad_positions[0], sizeof(float) * 4);
            memcpy(&dti.vertex_positions[1], &sas_quad_positions[1], sizeof(float) * 4);
            memcpy(&dti.vertex_positions[2], &sas_quad_positions[3], sizeof(float) * 4);

            dti.vertex_colors[0] = sas_quad_colors[0];
            dti.vertex_colors[1] = sas_quad_colors[1];
            dti.vertex_colors[2] = sas_quad_colors[3];

            memcpy(&dti.vertex_texcoords[0], &sas_quad_texcoords[0], sizeof(float) * 4);
            memcpy(&dti.vertex_texcoords[1], &sas_quad_texcoords[1], sizeof(float) * 4);
            memcpy(&dti.vertex_texcoords[2], &sas_quad_texcoords[3], sizeof(float) * 4);

            dti.vertex_indizes[0] = 0;
            dti.vertex_indizes[1] = 1;
            dti.vertex_indizes[2] = 3;

            sas_thread_execute(sas_do_triangle, &dti, sizeof(dti));


            memcpy(&dti.vertex_positions[0], &sas_quad_positions[3], sizeof(float) * 4);
            memcpy(&dti.vertex_positions[1], &sas_quad_positions[2], sizeof(float) * 4);
            memcpy(&dti.vertex_positions[2], &sas_quad_positions[0], sizeof(float) * 4);

            dti.vertex_colors[0] = sas_quad_colors[3];
            dti.vertex_colors[1] = sas_quad_colors[2];
            dti.vertex_colors[2] = sas_quad_colors[0];

            memcpy(&dti.vertex_texcoords[0], &sas_quad_texcoords[3], sizeof(float) * 4);
            memcpy(&dti.vertex_texcoords[1], &sas_quad_texcoords[2], sizeof(float) * 4);
            memcpy(&dti.vertex_texcoords[2], &sas_quad_texcoords[0], sizeof(float) * 4);

            dti.vertex_indizes[0] = 3;
            dti.vertex_indizes[1] = 2;
            dti.vertex_indizes[2] = 0;

            sas_thread_execute(sas_do_triangle, &dti, sizeof(dti));
#else
            sas_color_t curcol = sas_current_color;


            sas_do_triangle(sas_quad_colors[0], sas_quad_texcoords[0], sas_quad_positions[0], 0,
                            sas_quad_colors[1], sas_quad_texcoords[1], sas_quad_positions[1], 1,
                            sas_quad_colors[3], sas_quad_texcoords[3], sas_quad_positions[3], 3);

            sas_do_triangle(sas_quad_colors[3], sas_quad_texcoords[3], sas_quad_positions[3], 3,
                            sas_quad_colors[2], sas_quad_texcoords[2], sas_quad_positions[2], 2,
                            sas_quad_colors[0], sas_quad_texcoords[0], sas_quad_positions[0], 0);


            sas_current_color = curcol;
#endif

            sas_flush_varyings_partially(2);

            memcpy(&sas_quad_colors[0], &sas_quad_colors[2], sizeof(sas_quad_colors[0]) * 2);
            memcpy(&sas_quad_texcoords[0], &sas_quad_texcoords[2], sizeof(sas_quad_texcoords[0]) * 2);
            memcpy(&sas_quad_positions[0], &sas_quad_positions[2], sizeof(sas_quad_positions[0]) * 2);
        }
    }
}
