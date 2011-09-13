#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef THREADING
#include <pthread.h>
#endif

#include <soft-and-slow/context.h>
#include <soft-and-slow/helpers.h>
#include <soft-and-slow/light.h>
#include <soft-and-slow/limits.h>
#include <soft-and-slow/matrix.h>
#include <soft-and-slow/pipeline.h>
#include <soft-and-slow/types.h>


extern sas_color_t sas_texture_get(int unit, float u, float v);


extern sas_context_t current_sas_context;

extern SAS_MATRIX_TYPE sas_modelview[16], sas_projection[16];
extern SAS_MATRIX_TYPE sas_modelviewprojection[16], sas_normal_matrix[9];

extern SAS_DEPTH_TYPE sas_far_depth, sas_near_depth;

extern float sas_alpha_ref;

extern bool sas_do_alpha_test, sas_do_depth_test, sas_2d_textures_enabled, sas_lighting_enabled;

extern bool (*sas_depth_func)(float new, float current);
extern bool (*sas_alpha_func)(float new, float ref);

extern void (*sas_vertex_transformation)(void);
extern void (*sas_fragment_transformation)(sas_draw_thread_info_t *dti);

extern sas_color_t sas_current_color;
extern float sas_current_vertex[4], sas_current_position[4], sas_current_normal[3];

extern float sas_current_texcoord[8][4], sas_multi_texcoord0[4];

extern unsigned sas_current_buf_index;

extern sas_light_t sas_lights[SAS_LIGHTS];
extern sas_material_t sas_current_material;


#ifndef USE_SHADERS
static inline void normalize_3(float *vec)
{
    float len = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    if (len != 1.f)
    {
        len = 1.f / len;

        vec[0] *= len;
        vec[1] *= len;
        vec[2] *= len;
    }
}

static inline float dot_3(float *v1, float *v2)
{
    return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}
#endif


void sas_transform_vertex_to_screen(void)
{
#ifdef USE_SHADERS
    sas_vertex_transformation();
#else
    memcpy(sas_current_position, sas_current_vertex, sizeof(float) * 4);

    sas_matrix_dot_vector(sas_modelviewprojection, sas_current_position);

    memcpy(sas_current_texcoord[0], sas_multi_texcoord0, sizeof(float) * 4);

    if (sas_lighting_enabled)
    {
        sas_color_t col = (sas_color_t){ 0.f, 0.f, 0.f, 0.f };

        float normal[3];
        memcpy(normal, sas_current_normal, sizeof(float) * 3);

        sas_matrix_dot_vector_3x3(sas_normal_matrix, normal);

        normalize_3(normal);

        for (int i = 0; i < 8; i++)
        {
            if (!sas_lights[i].enabled)
                continue;

            float pos[3];
            memcpy(pos, sas_lights[i].position, sizeof(float) * 3);

            normalize_3(pos);

#ifdef USE_ASSEMBLY
            __asm__ __volatile__ ("movaps xmm15,%1;"
                                  "mulps xmm15,%2;"
                                  "addps xmm15,%3;"
                                  "movaps %0,xmm15"
                                  : "=m"(col)
                                  : "m"(sas_lights[i].ambient), "m"(sas_current_material.ambient), "m"(col)
                                  : "xmm15");
#else
            sas_color_t ambient = sas_lights[i].ambient;
            ambient.r *= sas_current_material.ambient.r;
            ambient.g *= sas_current_material.ambient.g;
            ambient.b *= sas_current_material.ambient.b;
            ambient.a *= sas_current_material.ambient.a;

            col.r += ambient.r;
            col.g += ambient.g;
            col.b += ambient.b;
            col.a += ambient.a;
#endif

            // TODO: Point lights
            if (!sas_lights[i].position[3])
            {
                float dotp = dot_3(normal, pos);
                if (dotp > 0.f)
                {
#ifdef USE_ASSEMBLY
                    __asm__ __volatile__ ("pshufd xmm15,%1,0x00;"
                                          "mulps xmm15,%2;"
                                          "mulps xmm15,%3;"
                                          "addps xmm15,%4;"
                                          "movaps %0,xmm15"
                                          : "=m"(col)
                                          : "x"(dotp), "m"(sas_lights[i].diffuse), "m"(sas_current_material.diffuse), "m"(col)
                                          : "xmm15");
#else
                    sas_color_t diffuse = sas_lights[i].diffuse;
                    diffuse.r *= dotp * sas_current_material.diffuse.r;
                    diffuse.g *= dotp * sas_current_material.diffuse.g;
                    diffuse.b *= dotp * sas_current_material.diffuse.b;
                    diffuse.a *= dotp * sas_current_material.diffuse.a;

                    col.r += diffuse.r;
                    col.g += diffuse.g;
                    col.b += diffuse.b;
                    col.a += diffuse.a;
#endif
                }
                // TODO: Specular

#ifdef USE_ASSEMBLY
                __asm__ __volatile__ ("movaps xmm15,%1;"
                                     "addps xmm15,%2;"
                                     "movaps %0,xmm15"
                                     : "=m"(col)
                                     : "m"(sas_current_material.emission), "m"(col)
                                     : "xmm15");
#else
                col.r += sas_current_material.emission.r;
                col.g += sas_current_material.emission.g;
                col.b += sas_current_material.emission.b;
                col.a += sas_current_material.emission.a;
#endif
            }
        }

        sas_current_color = col;
    }
#endif

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

#ifdef USE_SHADERS
    sas_draw_thread_info_t dummy_dti;

    memcpy(dummy_dti.current_position, sas_current_position, sizeof(float) * 4);
    dummy_dti.current_color = sas_current_color;
    memcpy(dummy_dti.current_texcoord[0], sas_current_texcoord[0], sizeof(float) * 4);

    sas_fragment_transformation(&dummy_dti);

    sas_blend_pixel(current_sas_context, sas_current_buf_index, dummy_dti.current_color);
#else
    if (sas_2d_textures_enabled)
    {
        sas_color_t cur_text = sas_texture_get(0, sas_current_texcoord[0][0], sas_current_texcoord[0][1]);

#ifdef USE_ASSEMBLY
        __asm__ __volatile__ ("movaps xmm15,%1;"
                              "mulps xmm15,%2;"
                              "movaps %0,xmm15"
                              : "=m"(sas_current_color)
                              : "m"(sas_current_color), "m"(cur_text)
                              : "xmm15");
#else
        sas_current_color.r *= cur_text.r;
        sas_current_color.g *= cur_text.g;
        sas_current_color.b *= cur_text.b;
        sas_current_color.a *= cur_text.a;
#endif
    }

    sas_blend_pixel(current_sas_context, sas_current_buf_index, sas_current_color);
#endif
}
#endif
