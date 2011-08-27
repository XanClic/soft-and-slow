#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include <soft-and-slow/context.h>
#include <soft-and-slow/helpers.h>
#include <soft-and-slow/matrix.h>
#include <soft-and-slow/pipeline.h>
#include <soft-and-slow/types.h>


extern sas_context_t current_sas_context;

extern SAS_MATRIX_TYPE sas_modelview[16], sas_projection[16];

extern SAS_DEPTH_TYPE sas_far_depth, sas_near_depth;

extern float sas_alpha_ref;

extern bool sas_do_alpha_test, sas_do_depth_test;

extern bool (*sas_depth_func)(float new, float current);
extern bool (*sas_alpha_func)(float new, float ref);

extern void (*sas_vertex_transformation)(void);
extern void (*sas_fragment_transformation)(void);

extern sas_color_t sas_current_color;
extern float sas_current_vertex[4], sas_current_position[4];


void sas_transform_vertex_to_screen(void)
{
    sas_vertex_transformation();

    sas_current_position[3] = 1.f / sas_current_position[3];
    sas_current_position[0] *= sas_current_position[3];
    sas_current_position[1] *= sas_current_position[3];
    sas_current_position[2] *= sas_current_position[3];
}

bool sas_depth_test(sas_context_t ctx, unsigned x, unsigned y, float d)
{
    SAS_DEPTH_TYPE *depth_ptr = &ctx->depthbuffer[y * ctx->width + x];

    if (sas_depth_func(d, *depth_ptr))
    {
        *depth_ptr = d;
        return true;
    }

    return false;
}

bool sas_alpha_test(float alpha)
{
    return sas_alpha_func(alpha, sas_alpha_ref);
}

void sas_blend_pixel(sas_context_t ctx, unsigned x, unsigned y, sas_color_t color)
{
    ctx->colorbuffer[(ctx->height - 1 - y) * ctx->width + x].r = lrintf(clampf(color.r) * 255.f);
    ctx->colorbuffer[(ctx->height - 1 - y) * ctx->width + x].g = lrintf(clampf(color.g) * 255.f);
    ctx->colorbuffer[(ctx->height - 1 - y) * ctx->width + x].b = lrintf(clampf(color.b) * 255.f);
    ctx->colorbuffer[(ctx->height - 1 - y) * ctx->width + x].a = lrintf(clampf(color.a) * 255.f);
}

void sas_transform_fragment(void)
{
    // Frustum Culling
    if ((sas_current_position[0] < -1.) || (sas_current_position[0] > 1.) ||
        (sas_current_position[1] < -1.) || (sas_current_position[1] > 1.) ||
        (sas_current_position[2] < -1.) || (sas_current_position[2] > 1.))
    {
        return;
    }


    sas_current_position[0] = ((sas_current_position[0] + 1.) / 2.) * current_sas_context->width;
    sas_current_position[1] = ((sas_current_position[1] + 1.) / 2.) * current_sas_context->height;

    // Bringt depth value to range [0; 1]
    float nd = sas_current_position[2] / 2. + .5;
    sas_current_position[2] = nd * sas_far_depth + (1. - nd) * sas_near_depth;


    if (sas_do_alpha_test && !sas_alpha_test(sas_current_color.a / 255.f))
        return;

    unsigned x = (unsigned)sas_current_position[0];
    unsigned y = (unsigned)sas_current_position[1];

    if (sas_do_depth_test && !sas_depth_test(current_sas_context, x, y, sas_current_position[2]))
        return;

    sas_fragment_transformation();

    sas_blend_pixel(current_sas_context, x, y, sas_current_color);
}
