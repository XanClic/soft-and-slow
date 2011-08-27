#include <stdbool.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/helpers.h>
#include <soft-and-slow/types.h>


extern GLenum sas_error;

extern float sas_alpha_ref;

extern bool (*sas_depth_func)(float new, float current);
extern bool (*sas_alpha_func)(float new, float ref);


static bool comp_always (float n, float r) { (void)n; (void)r; return true;  }
static bool comp_never  (float n, float r) { (void)n; (void)r; return false; }
static bool comp_less   (float n, float r) { return n <  r; }
static bool comp_lequal (float n, float r) { return n <= r; }
static bool comp_greater(float n, float r) { return n >  r; }
static bool comp_gequal (float n, float r) { return n >= r; }
static bool comp_equal  (float n, float r) { return n == r; }
static bool comp_nequal (float n, float r) { return n != r; }

static void set_comparison(bool (**func_ptr)(float new, float ref), GLenum func)
{
    switch (func)
    {
        case GL_ALWAYS:
            *func_ptr = &comp_always;
            break;
        case GL_NEVER:
            *func_ptr = &comp_never;
            break;
        case GL_LESS:
            *func_ptr = &comp_less;
            break;
        case GL_LEQUAL:
            *func_ptr = &comp_lequal;
            break;
        case GL_GREATER:
            *func_ptr = &comp_greater;
            break;
        case GL_GEQUAL:
            *func_ptr = &comp_gequal;
            break;
        case GL_EQUAL:
            *func_ptr = &comp_equal;
            break;
        case GL_NOTEQUAL:
            *func_ptr = &comp_nequal;
            break;
        default:
            sas_error = GL_INVALID_ENUM;
    }
}

void glDepthFunc(GLenum func)
{
    set_comparison(&sas_depth_func, func);
}

void glAlphaFunc(GLenum func, GLclampf ref)
{
    sas_alpha_ref = clampf(ref);

    set_comparison(&sas_alpha_func, func);
}
