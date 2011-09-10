#include <stdbool.h>
#include <stdlib.h>

#include <soft-and-slow/gl.h>
#include <soft-and-slow/limits.h>
#include <soft-and-slow/types.h>


extern GLenum sas_error;

extern sas_command_list_t **sas_current_command_list;
extern bool sas_execute_ccl;


typedef struct sas_display_list sas_display_list_t;

struct sas_display_list
{
    sas_command_list_t *cl;

    bool in_use;
};


static sas_display_list_t display_lists[SAS_MAX_DISPLAY_LISTS];

GLuint glGenLists(GLsizei range)
{
    if (range <= 0)
    {
        if (range < 0)
            sas_error = GL_INVALID_VALUE;
        return 0;
    }


    // FIXME: Atomically.
    for (int i = 0; i < SAS_MAX_DISPLAY_LISTS; i++)
    {
        if (!display_lists[i].in_use)
        {
            int j;
            for (j = 1; (j < range) && !display_lists[i + j].in_use && (i + j < SAS_MAX_DISPLAY_LISTS); j++);

            if (j < range)
            {
                i += j - 1;
                continue;
            }

            for (j = 0; j < range; j++)
                display_lists[i + j].in_use = true;

            return i + 1;
        }
    }

    return 0;
}

void glDeleteLists(GLuint list, GLsizei range)
{
    if (!list)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }

    if (range <= 0)
    {
        if (range < 0)
            sas_error = GL_INVALID_VALUE;
        return;
    }


    list--;

    for (int i = (int)list; i < (int)list + (int)range; i++)
    {
        if (display_lists[i].in_use)
        {
            sas_command_list_t *cl = display_lists[i].cl;
            while (cl != NULL)
            {
                sas_command_list_t *n = cl->next;

                if (cl->cmd == SAS_COMMAND_MATERIAL_V)
                    free(((struct sas_command_material_v *)cl)->param);

                free(cl);
                cl = n;
            }

            display_lists[i].cl = NULL;
            display_lists[i].in_use = false;
        }
    }
}


void glNewList(GLuint list, GLenum mode)
{
    if (!list || !display_lists[list - 1].in_use)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }

    if ((mode != GL_COMPILE) && (mode != GL_COMPILE_AND_EXECUTE))
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }


    sas_command_list_t *cl = display_lists[--list].cl;
    while (cl != NULL)
    {
        sas_command_list_t *n = cl->next;

        if (cl->cmd == SAS_COMMAND_MATERIAL_V)
            free(((struct sas_command_material_v *)cl)->param);

        free(cl);
        cl = n;
    }

    display_lists[list].cl = NULL;

    sas_current_command_list = &display_lists[list].cl;
    sas_execute_ccl = (mode == GL_COMPILE_AND_EXECUTE);
}

void glEndList(void)
{
    sas_current_command_list = NULL;
}


void glCallList(GLuint list)
{
    if (!list || !display_lists[list - 1].in_use || (display_lists[list - 1].cl == NULL))
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    sas_command_list_t *cl = display_lists[--list].cl;

    while (cl != NULL)
    {
        switch (cl->cmd)
        {
            case SAS_COMMAND_BEGIN:
                glBegin(((struct sas_command_begin *)cl)->mode);
                break;
            case SAS_COMMAND_END:
                glEnd();
                break;
            case SAS_COMMAND_VERTEX:
            {
                float x = ((struct sas_command_vertex *)cl)->position[0];
                float y = ((struct sas_command_vertex *)cl)->position[1];
                float z = ((struct sas_command_vertex *)cl)->position[2];
                float w = ((struct sas_command_vertex *)cl)->position[3];

                glVertex4f(x, y, z, w);
                break;
            }
            case SAS_COMMAND_COLOR:
            {
                sas_color_t col = ((struct sas_command_color *)cl)->color;

                glColor4f(col.r, col.g, col.b, col.a);
                break;
            }
            case SAS_COMMAND_TEXCOORD:
            {
                float s = ((struct sas_command_texcoord *)cl)->coord[0];
                float t = ((struct sas_command_texcoord *)cl)->coord[1];
                float p = ((struct sas_command_texcoord *)cl)->coord[2];
                float q = ((struct sas_command_texcoord *)cl)->coord[3];

                glTexCoord4f(s, t, p, q);
                break;
            }
            case SAS_COMMAND_NORMAL:
            {
                float x = ((struct sas_command_normal *)cl)->normal[0];
                float y = ((struct sas_command_normal *)cl)->normal[1];
                float z = ((struct sas_command_normal *)cl)->normal[2];

                glNormal3f(x, y, z);
                break;
            }
            case SAS_COMMAND_MATERIAL_S:
                glMaterialf(((struct sas_command_material_s *)cl)->face,
                            ((struct sas_command_material_s *)cl)->material_cmd,
                            ((struct sas_command_material_s *)cl)->param);
                break;
            case SAS_COMMAND_MATERIAL_V:
                glMaterialfv(((struct sas_command_material_v *)cl)->face,
                             ((struct sas_command_material_v *)cl)->material_cmd,
                             ((struct sas_command_material_v *)cl)->param);
                break;
            case SAS_COMMAND_SHADE_MODEL:
                glShadeModel(((struct sas_command_shade_model *)cl)->model);
                break;
        }


        cl = cl->next;
    }
}
