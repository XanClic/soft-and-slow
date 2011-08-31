#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/matrix.h>
#include <soft-and-slow/types.h>


extern GLenum sas_error;

extern SAS_MATRIX_TYPE sas_modelview[16], sas_projection[16], sas_modelviewprojection[16], sas_normal_matrix[9];
extern SAS_MATRIX_TYPE *sas_current_matrix;

extern struct sas_matrix_stack *sas_modelview_stack, *sas_projection_stack;
extern struct sas_matrix_stack **sas_current_matrix_stack;


void glMatrixMode(GLenum mode)
{
    switch (mode)
    {
        case GL_MODELVIEW:
            sas_current_matrix = sas_modelview;
            sas_current_matrix_stack = &sas_modelview_stack;
            break;
        case GL_PROJECTION:
            sas_current_matrix = sas_projection;
            sas_current_matrix_stack = &sas_projection_stack;
            break;
        default:
            sas_error = GL_INVALID_ENUM;
    }
}


// Updates the modelview-projection matrix
void sas_update_mvp(void)
{
    memcpy(sas_modelviewprojection, sas_projection, sizeof(sas_projection));
    sas_multiply_matrix(sas_modelviewprojection, sas_modelview);
}


void glLoadIdentity(void)
{
    memcpy(sas_current_matrix,
        (SAS_MATRIX_TYPE[16]){
            1., 0., 0., 0.,
            0., 1., 0., 0.,
            0., 0., 1., 0.,
            0., 0., 0., 1.
        },
        16 * sizeof(SAS_MATRIX_TYPE));

    sas_update_mvp();
}

void glLoadMatrixf(GLfloat *m)
{
    memcpy(sas_current_matrix, m, 16 * sizeof(SAS_MATRIX_TYPE));

    sas_update_mvp();
}

void glLoadMatrixd(GLdouble *m)
{
    for (int i = 0; i < 16; i++)
        sas_current_matrix[i] = m[i];

    sas_update_mvp();
}


void glPushMatrix(void)
{
    // Allocate a new stack element
    struct sas_matrix_stack *new_stack_element = malloc(sizeof(*new_stack_element));

    // Store the current matrix there
    new_stack_element->matrix = malloc(16 * sizeof(SAS_MATRIX_TYPE));
    memcpy(new_stack_element->matrix, sas_current_matrix, 16 * sizeof(SAS_MATRIX_TYPE));

    // And push that element on the current stack (FIXME: do it atomically)
    new_stack_element->next = *sas_current_matrix_stack;
    *sas_current_matrix_stack = new_stack_element;
}

void glPopMatrix(void)
{
    if (*sas_current_matrix_stack == NULL)
    {
        sas_error = GL_STACK_UNDERFLOW;
        return;
    }

    // Pop the top element off the current stack (FIXME: do it atomically)
    struct sas_matrix_stack *top_element = *sas_current_matrix_stack;
    *sas_current_matrix_stack = top_element->next;

    // Now restore the current matrix from there
    memcpy(sas_current_matrix, top_element->matrix, 16 * sizeof(SAS_MATRIX_TYPE));

    // And free the allocated data
    free(top_element->matrix);
    free(top_element);


    sas_update_mvp();
}


void glMultMatrixf(GLfloat *m)
{
    sas_multiply_matrix(sas_current_matrix, m);

    sas_update_mvp();
}

void glMultMatrixd(GLdouble *m)
{
    SAS_MATRIX_TYPE temp[16];

    for (int i = 0; i < 16; i++)
        temp[i] = m[i];

    sas_multiply_matrix(sas_current_matrix, temp);


    sas_update_mvp();
}

void sas_multiply_matrix(SAS_MATRIX_TYPE *d, const SAS_MATRIX_TYPE *s)
{
    SAS_MATRIX_TYPE nm[16];

    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 4; y++)
        {
            nm[x * 4 + y] = d[     y] * s[x * 4    ] +
                            d[ 4 + y] * s[x * 4 + 1] +
                            d[ 8 + y] * s[x * 4 + 2] +
                            d[12 + y] * s[x * 4 + 3];
        }
    }

    memcpy(d, nm, sizeof(nm));
}

void sas_multiply_matrix_3x3(SAS_MATRIX_TYPE *d, const SAS_MATRIX_TYPE *s)
{
    SAS_MATRIX_TYPE nm[9];

    for (int x = 0; x < 3; x++)
    {
        for (int y = 0; y < 3; y++)
        {
            nm[x * 3 + y] = d[     y] * s[x * 3    ] +
                            d[ 3 + y] * s[x * 3 + 1] +
                            d[ 6 + y] * s[x * 3 + 2];
        }
    }

    memcpy(d, nm, sizeof(nm));
}


void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    float len = sqrtf(x * x + y * y + z * z);

    if (len != 1)
    {
        x /= len;
        y /= len;
        z /= len;
    }

    angle *= (float)M_PI / 180.f;

    float s = sinf(angle);
    float c = cosf(angle);

    float omc = 1.f - c;


    // TODO: Optimizing by inlining
    SAS_MATRIX_TYPE rot_mat[16] = {
        x * x * omc +     c,   y * x * omc + z * s,   z * x * omc - y * s,   0.f,
        x * y * omc - z * s,   y * y * omc +     c,   z * y * omc + x * s,   0.f,
        x * z * omc + y * s,   y * z * omc - x * s,   z * z * omc +     c,   0.f,
                        0.f,                   0.f,                   0.f,   1.f
    };

    glMultMatrixf(rot_mat);


    // FIXME FIXME FIXME
    float tmp[9] = {
        x * x * omc +     c,   y * x * omc + z * s,   z * x * omc - y * s,
        x * y * omc - z * s,   y * y * omc +     c,   z * y * omc + x * s,
        x * z * omc + y * s,   y * z * omc - x * s,   z * z * omc +     c
    };

    sas_multiply_matrix_3x3(sas_normal_matrix, tmp);
}

void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    double len = sqrt(x * x + y * y + z * z);

    if (len != 1)
    {
        x /= len;
        y /= len;
        z /= len;
    }

    angle *= M_PI / 180.;

    double s = sin(angle);
    double c = cos(angle);

    double omc = 1. - c;


    SAS_MATRIX_TYPE rot_mat[16] = {
        x * x * omc +     c,   y * x * omc + z * s,   z * x * omc - y * s,   0.,
        x * y * omc - z * s,   y * y * omc +     c,   z * y * omc + x * s,   0.,
        x * z * omc + y * s,   y * z * omc - x * s,   z * z * omc +     c,   0.,
                         0.,                    0.,                    0.,   1.
    };

    glMultMatrixf(rot_mat);
}


#ifndef USE_ASSEMBLY
void glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    // Equals a multiplication by {{1,0,0,x},{0,1,0,y},{0,0,1,z},{0,0,0,1}}

    sas_current_matrix[12] += x * sas_current_matrix[0] + y * sas_current_matrix[4] + z * sas_current_matrix[ 8];
    sas_current_matrix[13] += x * sas_current_matrix[1] + y * sas_current_matrix[5] + z * sas_current_matrix[ 9];
    sas_current_matrix[14] += x * sas_current_matrix[2] + y * sas_current_matrix[6] + z * sas_current_matrix[10];
    sas_current_matrix[15] += x * sas_current_matrix[3] + y * sas_current_matrix[7] + z * sas_current_matrix[11];


    sas_update_mvp();
}

void glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    sas_current_matrix[12] += x * sas_current_matrix[0] + y * sas_current_matrix[4] + z * sas_current_matrix[ 8];
    sas_current_matrix[13] += x * sas_current_matrix[1] + y * sas_current_matrix[5] + z * sas_current_matrix[ 9];
    sas_current_matrix[14] += x * sas_current_matrix[2] + y * sas_current_matrix[6] + z * sas_current_matrix[10];
    sas_current_matrix[15] += x * sas_current_matrix[3] + y * sas_current_matrix[7] + z * sas_current_matrix[11];


    sas_update_mvp();
}


void glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    // Equals a multiplication by {{x,0,0,0},{0,y,0,0},{0,0,z,0},{0,0,0,1}}

    sas_current_matrix[0] *= x; sas_current_matrix[4] *= y; sas_current_matrix[ 8] *= z;
    sas_current_matrix[1] *= x; sas_current_matrix[5] *= y; sas_current_matrix[ 9] *= z;
    sas_current_matrix[2] *= x; sas_current_matrix[6] *= y; sas_current_matrix[10] *= z;
    sas_current_matrix[3] *= x; sas_current_matrix[7] *= y; sas_current_matrix[11] *= z;


    sas_update_mvp();
}

void glScaled(GLdouble x, GLdouble y, GLdouble z)
{
    sas_current_matrix[0] *= x; sas_current_matrix[4] *= y; sas_current_matrix[ 8] *= z;
    sas_current_matrix[1] *= x; sas_current_matrix[5] *= y; sas_current_matrix[ 9] *= z;
    sas_current_matrix[2] *= x; sas_current_matrix[6] *= y; sas_current_matrix[10] *= z;
    sas_current_matrix[3] *= x; sas_current_matrix[7] *= y; sas_current_matrix[11] *= z;


    sas_update_mvp();
}


void sas_matrix_dot_vector(const SAS_MATRIX_TYPE *matrix, SAS_MATRIX_TYPE *vector)
{
    SAS_MATRIX_TYPE x = matrix[0] * vector[0] + matrix[4] * vector[1] + matrix[ 8] * vector[2] + matrix[12] * vector[ 3];
    SAS_MATRIX_TYPE y = matrix[1] * vector[0] + matrix[5] * vector[1] + matrix[ 9] * vector[2] + matrix[13] * vector[ 3];
    SAS_MATRIX_TYPE z = matrix[2] * vector[0] + matrix[6] * vector[1] + matrix[10] * vector[2] + matrix[14] * vector[ 3];
    SAS_MATRIX_TYPE w = matrix[3] * vector[0] + matrix[7] * vector[1] + matrix[11] * vector[2] + matrix[15] * vector[ 3];

    vector[0] = x; vector[1] = y; vector[2] = z; vector[3] = w;
}

void sas_matrix_dot_vector_3x3(const SAS_MATRIX_TYPE *matrix, SAS_MATRIX_TYPE *vector)
{
    SAS_MATRIX_TYPE x = matrix[0] * vector[0] + matrix[3] * vector[1] + matrix[6] * vector[2];
    SAS_MATRIX_TYPE y = matrix[1] * vector[0] + matrix[4] * vector[1] + matrix[7] * vector[2];
    SAS_MATRIX_TYPE z = matrix[2] * vector[0] + matrix[5] * vector[1] + matrix[8] * vector[2];

    vector[0] = x; vector[1] = y; vector[2] = z;
}
#endif
