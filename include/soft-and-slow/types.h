#ifndef SAS_TYPES_H
#define SAS_TYPES_H

#include <stdint.h>

#include "compiler.h"


#define SAS_COLOR_TYPE   sas_icolor_t
#define SAS_DEPTH_TYPE   float
#define SAS_STENCIL_TYPE uint8_t

#define SAS_MATRIX_TYPE GLfloat
#define SAS_MATRIX_IS_FLOAT


typedef void          GLvoid;
typedef int           GLint;
typedef unsigned      GLuint;
typedef float         GLfloat;
typedef double        GLdouble;
typedef char          GLchar;
typedef unsigned char GLubyte;
typedef unsigned char GLboolean;
typedef int           GLsizei;
typedef unsigned      GLenum;
typedef unsigned      GLbitfield;
typedef float         GLclampf;
typedef double        GLclampd;


#ifndef __cplusplus
// C++ doesn't allow anonymous structs, thus we won't use them in that case
// (since S&S itself is C1X, that's OK -- this type should be irrelevant to
// exterior programs anyway).

// Integer color
typedef union sas_icolor sas_icolor_t;

union sas_icolor
{
    // Anonymous struct in order to access the red, green, blue and alpha
    // channel seperately
    struct
    {
        uint8_t b, g, r, a;
    } cc_packed;

    // Direct color value
    uint32_t c;
};
#else
typedef union sas_icolor sas_icolor_t;

union sas_icolor
{
    struct
    {
        uint8_t b, g, r, a;
    } channels;

    uint32_t c;
};
#endif


typedef struct sas_color sas_color_t;

struct sas_color
{
    float r, g, b, a;
};


// A 2D texture
typedef struct sas_texture_2d sas_texture_2d_t;

struct sas_texture_2d
{
    // Color data
    sas_color_t *data;

    unsigned width, height;
};


// Basic data types which may be assumed by varyings
enum sas_varying_types
{
    SAS_BOOL,
    SAS_INT,
    SAS_FLOAT,
    SAS_XMM
};


// One SSE register
typedef struct sas_xmm sas_xmm_t;

struct sas_xmm
{
    union
    {
        float    s[4];
        double   d[2];
        uint32_t i[4];
        uint8_t  b[16];
    };
} cc_packed;


typedef struct sas_draw_thread_info sas_draw_thread_info_t;

struct sas_draw_thread_info
{
    float current_position[4];
    sas_color_t current_color;
    float current_texcoord[8][4];

#ifdef THREADING
    float vertex_positions[3][4];
    sas_color_t vertex_colors[3];
    float vertex_texcoords[3][4];
    int vertex_indizes[3];


    unsigned current_buf_index;
#endif
};


#include "command-lists.h"
#include "light.h"


#endif
