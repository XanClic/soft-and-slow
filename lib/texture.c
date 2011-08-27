#include <stdbool.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/glext.h>
#include <soft-and-slow/types.h>


extern int sas_current_texture_unit;
extern bool sas_2d_textures_enabled;

extern GLenum sas_error;


void glActiveTexture(GLenum unit)
{
    int u = (int)unit - GL_TEXTURE0;

    if ((u < 0) || (u >= SAS_TEX_UNITS))
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }


    sas_current_texture_unit = u;

    glUniform1i(glGetUniformLocation(0, "display_texture"), u);
}

sas_color_t sas_texture_get(int unit, float u, float v)
{
    if (!sas_2d_textures_enabled || unit)
        return (sas_color_t){ .r = 1.f, .g = 1.f, .b = 1.f, .a = 1.f };

    if ((int)(u * 8.f) % 2 == (int)(v * 8.f) % 2)
        return (sas_color_t){ .r = .5f, .g = .5f, .b = .5f, .a = 1.f };
    else
        return (sas_color_t){ .r = 1.f, .g = 1.f, .b = 1.f, .a = 1.f };
}
