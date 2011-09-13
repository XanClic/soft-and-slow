#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/context.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/helpers.h>
#include <soft-and-slow/light.h>
#include <soft-and-slow/limits.h>
#include <soft-and-slow/matrix.h>
#include <soft-and-slow/types.h>


extern sas_context_t current_sas_context;

extern SAS_COLOR_TYPE sas_clear_color;
extern SAS_DEPTH_TYPE sas_clear_depth;
extern SAS_STENCIL_TYPE sas_clear_stencil;
extern GLenum sas_error;

extern SAS_MATRIX_TYPE sas_modelview[16], sas_projection[16];

extern bool sas_do_depth_test, sas_do_alpha_test, sas_2d_textures_enabled;
extern bool sas_smooth_shading, sas_lighting_enabled;
extern bool (*sas_depth_func)(float new, float current);
extern bool (*sas_alpha_func)(float new, float ref);

extern bool sas_do_cw_culling, sas_do_ccw_culling;
extern GLenum sas_cull_face, sas_front_face;

extern bool sas_normalize_normals;

extern sas_light_t sas_lights[SAS_LIGHTS];

extern sas_command_list_t **sas_current_command_list;
extern bool sas_execute_ccl;


extern void sas_load_fixed_pipeline(bool lighting);


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
        case GL_CULL_FACE:
            sas_do_cw_culling = sas_do_ccw_culling = false;
            switch (sas_cull_face)
            {
                case GL_FRONT_AND_BACK:
                    sas_do_cw_culling = sas_do_ccw_culling = true;
                    break;
                case GL_FRONT:
                    if (sas_front_face == GL_CW)
                        sas_do_cw_culling = true;
                    else
                        sas_do_ccw_culling = true;
                    break;
                case GL_BACK:
                    if (sas_front_face == GL_CW)
                        sas_do_ccw_culling = true;
                    else
                        sas_do_cw_culling = true;
                    break;
            }
            break;
        case GL_DEPTH_TEST:
            sas_do_depth_test = true;
            break;
        case GL_TEXTURE_2D:
            sas_2d_textures_enabled = true;
            break;
        case GL_LIGHTING:
#ifdef USE_SHADERS
            sas_load_fixed_pipeline(true);
#endif
            sas_lighting_enabled = true;
            break;
        case GL_LIGHT0:
            sas_lights[0].enabled = true;
            break;
        case GL_NORMALIZE:
            sas_normalize_normals = true;
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
        case GL_CULL_FACE:
            sas_do_cw_culling = sas_do_ccw_culling = false;
            break;
        case GL_DEPTH_TEST:
            sas_do_depth_test = false;
            break;
        case GL_TEXTURE_2D:
            sas_2d_textures_enabled = false;
            break;
        case GL_LIGHTING:
#ifdef USE_SHADERS
            sas_load_fixed_pipeline(false);
#endif
            sas_lighting_enabled = false;
            break;
        case GL_LIGHT0:
            sas_lights[0].enabled = false;
            break;
        case GL_NORMALIZE:
            sas_normalize_normals = false;
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
        case GL_CULL_FACE:
            return sas_do_cw_culling || sas_do_ccw_culling;
        case GL_DEPTH_TEST:
            return sas_do_depth_test;
        case GL_TEXTURE_2D:
            return sas_2d_textures_enabled;
        case GL_LIGHTING:
            return sas_lighting_enabled;
        case GL_LIGHT0:
            return sas_lights[0].enabled;
        case GL_NORMALIZE:
            return sas_normalize_normals;
        default:
            sas_error = GL_INVALID_ENUM;
    }

    return false;
}


GLenum glGetError(void)
{
    // FIXME: Should not return the last error but the first
    GLenum val = sas_error;
    sas_error = GL_NO_ERROR;

    return val;
}


void glCullFace(GLenum mode)
{
    if ((mode != GL_BACK) && (mode != GL_FRONT) && (mode != GL_FRONT_AND_BACK))
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }


    sas_cull_face = mode;

    // Reset modes
    if (glIsEnabled(GL_CULL_FACE))
        glEnable(GL_CULL_FACE);
}

void glFrontFace(GLenum mode)
{
    if ((mode != GL_CCW) && (mode != GL_CW))
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }


    sas_front_face = mode;

    // Reset modes
    if (glIsEnabled(GL_CULL_FACE))
        glEnable(GL_CULL_FACE);
}

void glShadeModel(GLenum mode)
{
    if (sas_current_command_list != NULL)
    {
        struct sas_command_shade_model *cmd_shm = calloc(1, sizeof(*cmd_shm));
        cmd_shm->cl.cmd = SAS_COMMAND_SHADE_MODEL;
        cmd_shm->model = mode;

        *sas_current_command_list = &cmd_shm->cl;
        sas_current_command_list = &cmd_shm->cl.next;

        if (!sas_execute_ccl)
            return;
    }


    if ((mode != GL_FLAT) && (mode != GL_SMOOTH))
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }


    sas_smooth_shading = (mode == GL_SMOOTH);
}

const GLubyte *glGetString(GLenum name)
{
    switch (name)
    {
        case GL_VENDOR:
            return (const GLubyte *)"ASX";
        case GL_RENDERER:
            return (const GLubyte *)"Soft And Slow";
        case GL_VERSION:
            return (const GLubyte *)"2.1";
        case GL_SHADING_LANGUAGE_VERSION:
            return (const GLubyte *)"1.40";
        case GL_EXTENSIONS:
            return (const GLubyte *)"GL_ARB_texture_non_power_of_two GL_ARB_texture_rectangle";
        default:
            sas_error = GL_INVALID_ENUM;
            return NULL;
    }
}
