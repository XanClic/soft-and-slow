#include <string.h>

#include <soft-and-slow/gl.h>
#include <soft-and-slow/light.h>
#include <soft-and-slow/limits.h>
#include <soft-and-slow/matrix.h>

extern GLenum sas_error;

extern sas_light_t sas_lights[SAS_LIGHTS];

extern sas_material_t sas_current_material;

extern SAS_MATRIX_TYPE sas_modelview[16];


void glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    if ((light < GL_LIGHT0) || (light - GL_LIGHT0 >= SAS_LIGHTS))
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }

    light -= GL_LIGHT0;


    switch (pname)
    {
        case GL_AMBIENT:
            memcpy(&sas_lights[light].ambient,  params, sizeof(sas_color_t));
            break;
        case GL_DIFFUSE:
            memcpy(&sas_lights[light].diffuse,  params, sizeof(sas_color_t));
            break;
        case GL_SPECULAR:
            memcpy(&sas_lights[light].specular, params, sizeof(sas_color_t));
            break;
        case GL_POSITION:
        {
            SAS_MATRIX_TYPE vec[4];
            memcpy(vec, params, sizeof(vec));
            sas_matrix_dot_vector(sas_modelview, vec);
            memcpy(&sas_lights[light].position, vec, sizeof(vec));
            break;
        }
        default:
            sas_error = GL_INVALID_ENUM;
    }
}


void glMaterialf(GLenum face, GLenum pname, const GLfloat param)
{
    // TODO
    (void)face;


    switch (pname)
    {
        case GL_SHININESS:
            sas_current_material.shininess = param;
            break;
        default:
            sas_error = GL_INVALID_ENUM;
    }
}

void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    // TODO
    (void)face;


    switch (pname)
    {
        case GL_AMBIENT:
            memcpy(&sas_current_material.ambient,  params, sizeof(sas_color_t));
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            memcpy(&sas_current_material.ambient,  params, sizeof(sas_color_t));
        case GL_DIFFUSE:
            memcpy(&sas_current_material.diffuse,  params, sizeof(sas_color_t));
            break;
        case GL_SPECULAR:
            memcpy(&sas_current_material.specular, params, sizeof(sas_color_t));
            break;
        case GL_EMISSION:
            memcpy(&sas_current_material.emission, params, sizeof(sas_color_t));
            break;
        default:
            sas_error = GL_INVALID_ENUM;
    }
}
