#include <math.h>
#include <stdio.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/context.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/helpers.h>
#include <soft-and-slow/pipeline.h>
#include <soft-and-slow/types.h>


extern sas_context_t current_sas_context;

extern GLenum sas_error;
extern GLenum sas_current_mode;

extern SAS_COLOR_TYPE sas_current_color;
extern float sas_multi_texcoord0[4];

extern bool sas_do_depth_test, sas_do_alpha_test;


void glBegin(GLenum mode)
{
    if (mode != GL_POINTS)
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }

    sas_current_mode = mode;
}

void glEnd(void)
{
    sas_current_mode = -1;
}


void glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
    // FIXME: Don't clamp yet
    sas_current_color.r = lrintf(clampf(r) * 255);
    sas_current_color.g = lrintf(clampf(g) * 255);
    sas_current_color.b = lrintf(clampf(b) * 255);
    sas_current_color.a = 1.f;
}

void glTexCoord2f(GLfloat s, GLfloat v)
{
    sas_multi_texcoord0[0] = s;
    sas_multi_texcoord0[1] = v;
    sas_multi_texcoord0[2] = 0.f;
    sas_multi_texcoord0[3] = 1.f;
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    SAS_MATRIX_TYPE vec[4] = { x, y, z, 1. };
    sas_wnd_coord_t wc;

    if (!sas_transform_vertex_to_screen(current_sas_context, vec, &wc))
        return;

    if (sas_do_alpha_test && !sas_alpha_test(sas_current_color.a / 255.f))
        return;

    if (sas_do_depth_test && !sas_depth_test(current_sas_context, wc))
        return;

    sas_blend_pixel(current_sas_context, wc, sas_current_color);
}
