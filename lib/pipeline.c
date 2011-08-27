#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include <soft-and-slow/context.h>
#include <soft-and-slow/matrix.h>
#include <soft-and-slow/pipeline.h>
#include <soft-and-slow/types.h>


extern SAS_MATRIX_TYPE sas_modelview[16], sas_projection[16];

extern SAS_DEPTH_TYPE sas_far_depth, sas_near_depth;

extern float sas_alpha_ref;

extern bool (*sas_depth_func)(float new, float current);
extern bool (*sas_alpha_func)(float new, float ref);

extern void (*sas_vertex_transformation)(void);

extern float sas_current_vertex[4], sas_current_position[4];


bool sas_transform_vertex_to_screen(sas_context_t ctx, const SAS_MATRIX_TYPE *vec, sas_wnd_coord_t *wc)
{
    for (int i = 0; i < 4; i++)
        sas_current_vertex[i] = vec[i];

    sas_vertex_transformation();

    sas_current_position[0] /= sas_current_position[3];
    sas_current_position[1] /= sas_current_position[3];
    sas_current_position[2] /= sas_current_position[3];


    // Frustum Culling
    if ((sas_current_position[0] < -1.) || (sas_current_position[0] > 1.) ||
        (sas_current_position[1] < -1.) || (sas_current_position[1] > 1.) ||
        (sas_current_position[2] < -1.) || (sas_current_position[2] > 1.))
    {
        return false;
    }


    wc->x = lrint(((sas_current_position[0] + 1.) / 2.) * ctx->width);
    wc->y = lrint(((1. - sas_current_position[1]) / 2.) * ctx->height);

    wc->d = sas_current_position[2] / 2. + .5;
    wc->d = wc->d * sas_far_depth + (1. - wc->d) * sas_near_depth;

    return true;
}

bool sas_depth_test(sas_context_t ctx, sas_wnd_coord_t wc)
{
    SAS_DEPTH_TYPE *depth_ptr = &ctx->depthbuffer[wc.y * ctx->width + wc.x];

    if (sas_depth_func(wc.d, *depth_ptr))
    {
        *depth_ptr = wc.d;
        return true;
    }

    return false;
}

bool sas_alpha_test(float alpha)
{
    return sas_alpha_func(alpha, sas_alpha_ref);
}

void sas_blend_pixel(sas_context_t ctx, sas_wnd_coord_t wc, sas_color_t color)
{
    ctx->colorbuffer[wc.y * ctx->width + wc.x].c = color.c;
}
