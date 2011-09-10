#include <string.h>

#include <soft-and-slow/gl.h>
#include <soft-and-slow/light.h>
#include <soft-and-slow/limits.h>
#include <soft-and-slow/matrix.h>

extern GLenum sas_error;

extern sas_light_t sas_lights[SAS_LIGHTS];

extern sas_material_t sas_current_material;

extern SAS_MATRIX_TYPE sas_modelview[16];

extern sas_command_list_t **sas_current_command_list;
extern bool sas_execute_ccl;


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
    if (sas_current_command_list != NULL)
    {
        struct sas_command_material_s *cmd_mat = calloc(1, sizeof(*cmd_mat));
        cmd_mat->cl.cmd = SAS_COMMAND_MATERIAL_S;
        cmd_mat->face = face;
        cmd_mat->material_cmd = pname;
        cmd_mat->param = param;

        *sas_current_command_list = &cmd_mat->cl;
        sas_current_command_list = &cmd_mat->cl.next;

        if (!sas_execute_ccl)
            return;
    }


    // TODO: face

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
    if (sas_current_command_list != NULL)
    {
        struct sas_command_material_v *cmd_mat = calloc(1, sizeof(*cmd_mat));
        cmd_mat->cl.cmd = SAS_COMMAND_MATERIAL_V;
        cmd_mat->face = face;
        cmd_mat->material_cmd = pname;
        // FIXME: size depends on pname (but works for now, because they are no
        // pnames supported which use other sizes).
        cmd_mat->param = malloc(sizeof(sas_color_t));
        memcpy(cmd_mat->param, params, sizeof(sas_color_t));

        *sas_current_command_list = &cmd_mat->cl;
        sas_current_command_list = &cmd_mat->cl.next;

        if (!sas_execute_ccl)
            return;
    }


    // TODO: face

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
