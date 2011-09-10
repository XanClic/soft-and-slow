#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef THREADING
#include <pthread.h>
#endif

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
extern void (*sas_fragment_transformation)(sas_draw_thread_info_t *dti);

extern sas_color_t sas_current_color;
extern float sas_current_vertex[4], sas_current_position[4];

extern float sas_current_texcoord[8][4];

extern unsigned sas_current_buf_index;


void sas_transform_vertex_to_screen(void)
{
    sas_vertex_transformation();

    sas_current_position[3] = 1.f / sas_current_position[3];
    sas_current_position[0] *= sas_current_position[3];
    sas_current_position[1] *= sas_current_position[3];
    sas_current_position[2] *= sas_current_position[3];
}

bool sas_depth_test(sas_context_t ctx, unsigned i, float d)
{
#ifdef THREADING
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
#endif


    SAS_DEPTH_TYPE *depth_ptr = &ctx->depthbuffer[i];

#ifdef THREADING
    pthread_mutex_lock(&lock);
    bool depth_result = sas_depth_func(d, *depth_ptr);
    pthread_mutex_unlock(&lock);

    if (depth_result)
#else
    if (sas_depth_func(d, *depth_ptr))
#endif
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

void sas_blend_pixel(sas_context_t ctx, unsigned i, sas_color_t color)
{
    SAS_COLOR_TYPE *o = &ctx->colorbuffer[i];

#ifdef USE_ASSEMBLY
    float col_max = 255.f;

    __asm__ __volatile__ ("xorps  xmm6,xmm6;" // 0
                          "maxps  xmm6,%4;" // clamp to [0; ∞[
                          "pshufd xmm7,%5,0x00;" // 255
                          "mulps  xmm6,xmm7;"
                          "minps  xmm6,xmm7;" // clamp to [0; 255]
                          "cvtss2si eax,xmm6;"
                          "shufps xmm6,xmm6,0xE5;"
                          "cvtss2si edi,xmm6;"
                          "shufps xmm6,xmm6,0xE6;"
                          "cvtss2si ecx,xmm6;"
                          "shufps xmm6,xmm6,0xE7;"
                          "cvtss2si edx,xmm6"
                          : "=a"(o->r), "=D"(o->g), "=c"(o->b), "=d"(o->a)
                          : "m"(color), "x"(col_max));
#else
    o->r = lrintf(clampf(color.r) * 255.f);
    o->g = lrintf(clampf(color.g) * 255.f);
    o->b = lrintf(clampf(color.b) * 255.f);
    o->a = lrintf(clampf(color.a) * 255.f);
#endif
}


#ifdef THREADING
void sas_transform_fragment(sas_draw_thread_info_t *dti)
{
    // FIXME: Review locking (nothing here, but in sas_depth_test)

    dti->current_position[2] = dti->current_position[2] * sas_far_depth + (1.f - dti->current_position[2]) * sas_near_depth;


    if (sas_do_alpha_test && !sas_alpha_test(dti->current_color.a))
        return;

    if (sas_do_depth_test && !sas_depth_test(current_sas_context, dti->current_buf_index, dti->current_position[2]))
        return;

    sas_fragment_transformation(dti);

    sas_blend_pixel(current_sas_context, dti->current_buf_index, dti->current_color);
}
#else
void sas_transform_fragment(void)
{
    sas_current_position[2] = sas_current_position[2] * sas_far_depth + (1.f - sas_current_position[2]) * sas_near_depth;


    if (sas_do_alpha_test && !sas_alpha_test(sas_current_color.a))
        return;

    if (sas_do_depth_test && !sas_depth_test(current_sas_context, sas_current_buf_index, sas_current_position[2]))
        return;

    sas_draw_thread_info_t dummy_dti;

    memcpy(dummy_dti.current_position, sas_current_position, sizeof(float) * 4);
    dummy_dti.current_color = sas_current_color;
    memcpy(dummy_dti.current_texcoord[0], sas_current_texcoord[0], sizeof(float) * 4);

    sas_fragment_transformation(&dummy_dti);

    sas_blend_pixel(current_sas_context, sas_current_buf_index, dummy_dti.current_color);
}
#endif
