#include <math.h>
#include <stdio.h>
#include <string.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/context.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/helpers.h>
#include <soft-and-slow/pipeline.h>
#include <soft-and-slow/types.h>


extern sas_context_t current_sas_context;

extern GLenum sas_error;
extern GLenum sas_current_mode;

extern sas_color_t sas_current_color;
extern float sas_multi_texcoord0[4], sas_current_vertex[4];

extern float sas_current_position[4], sas_triangle_positions[3][4];
extern float sas_triangle_texcoords[3][4], sas_current_texcoord[8][4];
extern sas_color_t sas_triangle_colors[3];

extern int sas_triangle_index;


void glBegin(GLenum mode)
{
    switch (mode)
    {
        case GL_POINTS:
            sas_current_mode = GL_POINTS;
            break;
        case GL_TRIANGLES:
            sas_triangle_index = 0;
            sas_current_mode = GL_TRIANGLES;
            break;
        default:
            sas_error = GL_INVALID_ENUM;
    }
}

void glEnd(void)
{
    sas_current_mode = -1;
}


void glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
    // FIXME: Don't clamp yet
    sas_current_color.r = r;
    sas_current_color.g = g;
    sas_current_color.b = b;
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
    sas_current_vertex[0] = x;
    sas_current_vertex[1] = y;
    sas_current_vertex[2] = z;
    sas_current_vertex[3] = 1.f;

    if (!sas_transform_vertex_to_screen())
        return;

    if (sas_current_mode == GL_POINTS)
        sas_transform_fragment();
    else
    {
        // GL_TRIANGLES

        sas_triangle_colors[sas_triangle_index] = sas_current_color;
        memcpy(sas_triangle_texcoords[sas_triangle_index], sas_multi_texcoord0, sizeof(sas_multi_texcoord0));
        memcpy(sas_triangle_positions[sas_triangle_index++], sas_current_position, sizeof(sas_current_position));

        if (sas_triangle_index == 3)
        {
            sas_triangle_index = 0;

            float vec1[4] = {
                sas_triangle_positions[1][0] - sas_triangle_positions[0][0],
                sas_triangle_positions[1][1] - sas_triangle_positions[0][1],
                sas_triangle_positions[1][2] - sas_triangle_positions[0][2],
                sas_triangle_positions[1][3] - sas_triangle_positions[0][3]
            };

            float vec2[4] = {
                sas_triangle_positions[2][0] - sas_triangle_positions[0][0],
                sas_triangle_positions[2][1] - sas_triangle_positions[0][1],
                sas_triangle_positions[2][2] - sas_triangle_positions[0][2],
                sas_triangle_positions[2][3] - sas_triangle_positions[0][3]
            };

            float vec3[4] = {
                sas_triangle_positions[2][0] - sas_triangle_positions[1][0],
                sas_triangle_positions[2][1] - sas_triangle_positions[1][1],
                sas_triangle_positions[2][2] - sas_triangle_positions[1][2],
                sas_triangle_positions[2][3] - sas_triangle_positions[1][3]
            };


            float unit1 = 1.f / sqrtf(vec1[0] * vec1[0] * current_sas_context->width * current_sas_context->width + vec1[1] * vec1[1] * current_sas_context->height * current_sas_context->height);
            float unit2 = 1.f / sqrtf(vec2[0] * vec2[0] * current_sas_context->width * current_sas_context->width + vec2[1] * vec2[1] * current_sas_context->height * current_sas_context->height);

            for (float s = 0.f; s < 1.f; s += unit1)
            {
                for (float t = 0.f; t < 1.f - s; t += unit2)
                {
                    sas_current_position[0] = sas_triangle_positions[0][0] + s * vec1[0] + t * vec2[0];
                    sas_current_position[1] = sas_triangle_positions[0][1] + s * vec1[1] + t * vec2[1];
                    sas_current_position[2] = sas_triangle_positions[0][2] + s * vec1[2] + t * vec2[2];
                    sas_current_position[3] = sas_triangle_positions[0][3] + s * vec1[3] + t * vec2[3];

                    // Distance from the respectice corners
                    float d0 = s + t;
                    float d1 = ((-vec1[0] - vec3[0]) * (sas_triangle_positions[1][1] - sas_current_position[1]) + ( vec3[1] + vec1[1]) * (sas_triangle_positions[1][0] - sas_current_position[0])) / (vec3[1] * vec1[0] - vec1[1] * vec3[0]);
                    float d2 = ((-vec2[0] + vec3[0]) * (sas_triangle_positions[2][1] - sas_current_position[1]) + (-vec3[1] + vec2[1]) * (sas_triangle_positions[2][0] - sas_current_position[0])) / (vec2[1] * vec3[0] - vec3[1] * vec2[0]);

                    d0 = 1.f - d0;
                    d1 = 1.f - d1;
                    d2 = 1.f - d2;

                    sas_current_color.r = sas_triangle_colors[0].r * d0 + sas_triangle_colors[1].r * d1 + sas_triangle_colors[2].r * d2;
                    sas_current_color.g = sas_triangle_colors[0].g * d0 + sas_triangle_colors[1].g * d1 + sas_triangle_colors[2].g * d2;
                    sas_current_color.b = sas_triangle_colors[0].b * d0 + sas_triangle_colors[1].b * d1 + sas_triangle_colors[2].b * d2;
                    sas_current_color.a = sas_triangle_colors[0].a * d0 + sas_triangle_colors[1].a * d1 + sas_triangle_colors[2].a * d2;

                    sas_current_texcoord[0][0] = sas_triangle_texcoords[0][0] * d0 + sas_triangle_texcoords[1][0] * d1 + sas_triangle_texcoords[2][0] * d2;
                    sas_current_texcoord[0][1] = sas_triangle_texcoords[0][1] * d0 + sas_triangle_texcoords[1][1] * d1 + sas_triangle_texcoords[2][1] * d2;
                    sas_current_texcoord[0][2] = sas_triangle_texcoords[0][2] * d0 + sas_triangle_texcoords[1][2] * d1 + sas_triangle_texcoords[2][2] * d2;
                    sas_current_texcoord[0][3] = sas_triangle_texcoords[0][3] * d0 + sas_triangle_texcoords[1][3] * d1 + sas_triangle_texcoords[2][3] * d2;

                    sas_transform_fragment();
                }
            }
        }
    }
}
