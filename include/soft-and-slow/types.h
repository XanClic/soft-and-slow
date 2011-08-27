#ifndef SAS_TYPES_H
#define SAS_TYPES_H

#include <stdint.h>


#define SAS_COLOR_TYPE   sas_color_t
#define SAS_DEPTH_TYPE   float
#define SAS_STENCIL_TYPE uint8_t

#define SAS_MATRIX_TYPE GLfloat
#define SAS_MATRIX_IS_FLOAT


typedef int           GLint;
typedef unsigned      GLuint;
typedef float         GLfloat;
typedef double        GLdouble;
typedef char          GLchar;
typedef unsigned char GLboolean;
typedef int           GLsizei;
typedef unsigned      GLenum;
typedef unsigned      GLbitfield;
typedef float         GLclampf;
typedef double        GLclampd;


typedef union sas_color sas_color_t;

union sas_color
{
    // Anonymous struct in order to access the red, green, blue and alpha
    // channel seperately
    struct
    {
        uint8_t r, g, b, a;
    } cc_packed;

    // Direct color value
    uint32_t c;
};


// Defines window coordinates
typedef struct sas_wnd_coord sas_wnd_coord_t;

struct sas_wnd_coord
{
    unsigned x, y;

    SAS_DEPTH_TYPE d;
};

#endif
