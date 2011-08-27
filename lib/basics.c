#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/context.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/helpers.h>
#include <soft-and-slow/matrix.h>
#include <soft-and-slow/types.h>


extern sas_context_t current_sas_context;

extern SAS_COLOR_TYPE sas_clear_color;
extern SAS_DEPTH_TYPE sas_clear_depth;
extern SAS_STENCIL_TYPE sas_clear_stencil;
extern GLenum sas_error;

extern SAS_MATRIX_TYPE sas_modelview[16], sas_projection[16];

extern bool sas_do_depth_test, sas_do_alpha_test, sas_2d_textures_enabled;
extern bool (*sas_depth_func)(float new, float current);
extern bool (*sas_alpha_func)(float new, float ref);


void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a)
{
    sas_clear_color.r = lrintf(clampf(r) * 255.f);
    sas_clear_color.g = lrintf(clampf(g) * 255.f);
    sas_clear_color.b = lrintf(clampf(b) * 255.f);
    sas_clear_color.a = lrintf(clampf(a) * 255.f);
}

void glClearDepth(GLclampd d)
{
    sas_clear_depth = clampd(d);
}

void glClearStencil(GLint s)
{
    sas_clear_stencil = s;
}


void glClear(GLbitfield mask)
{
    if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }

    size_t bufsz = current_sas_context->width * current_sas_context->height;

    if (mask & GL_COLOR_BUFFER_BIT)
        memset_color(current_sas_context->colorbuffer, sas_clear_color, bufsz);

    if (mask & GL_DEPTH_BUFFER_BIT)
        memset_depth(current_sas_context->depthbuffer, sas_clear_depth, bufsz);

    if (mask & GL_STENCIL_BUFFER_BIT)
        memset_stencil(current_sas_context->stencilbuffer, sas_clear_stencil, bufsz);
}


void glGetFloatv(GLenum pname, float *params)
{
    switch (pname)
    {
        case GL_MODELVIEW_MATRIX:
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                    params[x * 4 + y] = sas_modelview[y * 4 + x];
            break;
        case GL_PROJECTION_MATRIX:
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                    params[x * 4 + y] = sas_projection[y * 4 + x];
            break;
        default:
            sas_error = GL_INVALID_ENUM;
    }
}


void glEnable(GLenum cap)
{
    switch (cap)
    {
        case GL_ALPHA_TEST:
            sas_do_alpha_test = true;
            break;
        case GL_DEPTH_TEST:
            sas_do_depth_test = true;
            break;
        case GL_TEXTURE_2D:
            sas_2d_textures_enabled = true;
            break;
        default:
            sas_error = GL_INVALID_ENUM;
    }
}

void glDisable(GLenum cap)
{
    switch (cap)
    {
        case GL_ALPHA_TEST:
            sas_do_alpha_test = false;
            break;
        case GL_DEPTH_TEST:
            sas_do_depth_test = false;
            break;
        case GL_TEXTURE_2D:
            sas_2d_textures_enabled = false;
            break;
        default:
            sas_error = GL_INVALID_ENUM;
    }
}

GLboolean glIsEnabled(GLenum cap)
{
    switch (cap)
    {
        case GL_ALPHA_TEST:
            return sas_do_alpha_test;
        case GL_DEPTH_TEST:
            return sas_do_depth_test;
        case GL_TEXTURE_2D:
            return sas_2d_textures_enabled;
        default:
            sas_error = GL_INVALID_ENUM;
    }

    return false;
}
