#include <stdbool.h>
#include <stdlib.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/glext.h>
#include <soft-and-slow/helpers.h>
#include <soft-and-slow/limits.h>
#include <soft-and-slow/types.h>


extern int sas_current_texture_unit;
extern bool sas_2d_textures_enabled;

extern GLenum sas_error;

extern sas_texture_2d_t *sas_textures_2d[SAS_TEXTURES];
extern GLuint sas_texture_units_2d[SAS_TEX_UNITS];


void glActiveTexture(GLenum unit)
{
    int u = (int)unit - GL_TEXTURE0;

    if ((u < 0) || (u >= SAS_TEX_UNITS))
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }


    sas_current_texture_unit = u;
}

sas_color_t sas_texture_get(int unit, float u, float v)
{
    sas_texture_2d_t *t = sas_textures_2d[sas_texture_units_2d[unit]];

    if (!sas_2d_textures_enabled || (t == NULL) || (t->data == NULL))
        return (sas_color_t){ .r = 1.f, .g = 1.f, .b = 1.f, .a = 1.f };


    u = clampf(u);
    v = clampf(v);


    unsigned x = (u == 1.f) ? (t->width  - 1) : (unsigned)(u * t->width );
    unsigned y = (v == 1.f) ? (t->height - 1) : (unsigned)(v * t->height);

    return t->data[y * t->width + x];
}


// Color and value transform operations
static sas_color_t get_color_red            (const void **pixel, float (*get_value_op)(const void **subpixel))
{ return (sas_color_t){ .r = get_value_op(pixel), .g = 0.f, .b = 0.f, .a = 1.f }; }
static sas_color_t get_color_green          (const void **pixel, float (*get_value_op)(const void **subpixel))
{ return (sas_color_t){ .r = 0.f, .g = get_value_op(pixel), .b = 0.f, .a = 1.f }; }
static sas_color_t get_color_blue           (const void **pixel, float (*get_value_op)(const void **subpixel))
{ return (sas_color_t){ .r = 0.f, .g = 0.f, .b = get_value_op(pixel), .a = 1.f }; }
static sas_color_t get_color_alpha          (const void **pixel, float (*get_value_op)(const void **subpixel))
{ return (sas_color_t){ .r = 0.f, .g = 0.f, .b = 0.f, .a = get_value_op(pixel) }; }
static sas_color_t get_color_intensity      (const void **pixel, float (*get_value_op)(const void **subpixel))
{ float i = get_value_op(pixel); return (sas_color_t){ .r = i, .g = i, .b = i, .a = i }; }
static sas_color_t get_color_luminance      (const void **pixel, float (*get_value_op)(const void **subpixel))
{ float i = get_value_op(pixel); return (sas_color_t){ .r = i, .g = i, .b = i, .a = 1.f }; }
static sas_color_t get_color_luminance_alpha(const void **pixel, float (*get_value_op)(const void **subpixel))
{ float i = get_value_op(pixel); return (sas_color_t){ .r = i, .g = i, .b = i, .a = get_value_op(pixel) }; }
static sas_color_t get_color_rgb            (const void **pixel, float (*get_value_op)(const void **subpixel))
{ return (sas_color_t){ .r = get_value_op(pixel), .g = get_value_op(pixel), .b = get_value_op(pixel), .a = 1.f }; }
static sas_color_t get_color_bgr            (const void **pixel, float (*get_value_op)(const void **subpixel))
{ return (sas_color_t){ .b = get_value_op(pixel), .g = get_value_op(pixel), .r = get_value_op(pixel), .a = 1.f }; }
static sas_color_t get_color_rgba           (const void **pixel, float (*get_value_op)(const void **subpixel))
{ return (sas_color_t){ .r = get_value_op(pixel), .g = get_value_op(pixel), .b = get_value_op(pixel), .a = get_value_op(pixel) }; }
static sas_color_t get_color_bgra           (const void **pixel, float (*get_value_op)(const void **subpixel))
{ return (sas_color_t){ .b = get_value_op(pixel), .g = get_value_op(pixel), .r = get_value_op(pixel), .a = get_value_op(pixel) }; }

static float get_val_u8    (const void **subpixel)
{ float v = **(uint8_t *const *)subpixel / 255.f; *subpixel = (const void *)((uint8_t *)*subpixel + 1); return v; }
static float get_val_s8    (const void **subpixel)
{ float v = ((int16_t)**(int8_t *const *)subpixel + 128) / 255.f; *subpixel = (const void *)((int8_t *)*subpixel + 1); return v; }
static float get_val_u16   (const void **subpixel)
{ float v = **(uint16_t *const *)subpixel / 65535.f; *subpixel = (const void *)((uint16_t *)*subpixel + 1); return v; }
static float get_val_s16   (const void **subpixel)
{ float v = ((int32_t)**(int16_t *const *)subpixel + 32768) / 65535.f; *subpixel = (const void *)((int16_t *)*subpixel + 1); return v; }
static float get_val_u32   (const void **subpixel)
{ float v = **(uint32_t *const *)subpixel / 4294967295.f; *subpixel = (const void *)((uint32_t *)*subpixel + 1); return v; }
static float get_val_s32   (const void **subpixel)
{ float v = ((int64_t)**(int32_t *const *)subpixel + 2147483648) / 4294967295.f; *subpixel = (const void *)((int32_t *)*subpixel + 1); return v; }
static float get_val_single(const void **subpixel)
{ float v = **(float *const *)subpixel; *subpixel = (const void *)((float *)*subpixel + 1); return v; }


static float (*get_get_value_op(GLenum type))(const void **subpixel)
{
    switch (type)
    {
        case GL_UNSIGNED_BYTE:
            return &get_val_u8;
        case GL_BYTE:
            return &get_val_s8;
        case GL_UNSIGNED_SHORT:
            return &get_val_u16;
        case GL_SHORT:
            return &get_val_s16;
        case GL_UNSIGNED_INT:
            return &get_val_u32;
        case GL_INT:
            return &get_val_s32;
        case GL_FLOAT:
            return &get_val_single;
        default:
            // TODO
            return NULL;
    }
}

// I do love extremely complicated definitions.
static sas_color_t (*get_get_color_op(GLenum format))(const void **pixel, float (*get_value_op)(const void **subpixel))
{
    switch (format)
    {
        case GL_RED:
            return &get_color_red;
        case GL_GREEN:
            return &get_color_green;
        case GL_BLUE:
            return &get_color_blue;
        case GL_ALPHA:
            return &get_color_alpha;
        case GL_RGB:
            return &get_color_rgb;
        case GL_BGR:
            return &get_color_bgr;
        case GL_RGBA:
            return &get_color_rgba;
        case GL_BGRA:
            return &get_color_bgra;
        case GL_INTENSITY:
            return &get_color_intensity;
        case GL_LUMINANCE:
            return &get_color_luminance;
        case GL_LUMINANCE_ALPHA:
            return &get_color_luminance_alpha;
        default:
            return NULL;
    }
}


void glTexImage2D(GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *data)
{
    (void)internal_format;


    if (target != GL_TEXTURE_2D)
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }

    if (level || (width < 0) || (height < 0) || border)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    sas_texture_2d_t *t = sas_textures_2d[sas_texture_units_2d[sas_current_texture_unit]];
    if (t == NULL)
    {
        // TODO: Is that true?
        sas_error = GL_INVALID_OPERATION;
        return;
    }


    float (*get_value_op)(const void **subpixel) = get_get_value_op(type);
    sas_color_t (*get_color_op)(const void **pixel, float (*get_value_op)(const void **subpixel)) = get_get_color_op(format);

    if ((get_value_op == NULL) || (get_color_op == NULL))
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }


    free(t->data);

    t->data = malloc(width * height * sizeof(sas_color_t));
    t->width = width;
    t->height = height;


    if (data == NULL)
        return;


    // FIXME: Something about GL_c_BIAS and GL_c_SCALE
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            t->data[y * width + x] = get_color_op(&data, get_value_op);
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *data)
{
    if (target != GL_TEXTURE_2D)
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }

    if (level || (width < 0) || (height < 0) || (xoffset < 0) || (yoffset < 0))
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    sas_texture_2d_t *t = sas_textures_2d[sas_texture_units_2d[sas_current_texture_unit]];
    if ((t == NULL) || (t->data == NULL))
    {
        sas_error = GL_INVALID_OPERATION;
        return;
    }


    if ((xoffset + width > (int)t->width) || (yoffset + height > (int)t->height))
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    float (*get_value_op)(const void **subpixel) = get_get_value_op(type);
    sas_color_t (*get_color_op)(const void **pixel, float (*get_value_op)(const void **subpixel)) = get_get_color_op(format);

    if ((get_value_op == NULL) || (get_color_op == NULL))
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }


    for (int y = yoffset; y < yoffset + height; y++)
        for (int x = xoffset; x < xoffset + width; x++)
            t->data[y * width + x] = get_color_op(&data, get_value_op);
}


void glGenTextures(GLsizei n, GLuint *textures)
{
    if (n < 0)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    for (int i = 0; (i < SAS_TEXTURES) && n; i++)
    {
        // FIXME: Atomically
        if (sas_textures_2d[i] == NULL)
        {
            sas_textures_2d[i] = calloc(1, sizeof(sas_texture_2d_t));

            textures[--n] = i + 1;
        }
    }


    if (n)
    {
        sas_error = GL_OUT_OF_MEMORY;

        while (n)
            textures[--n] = 0;
    }
}


void glBindTexture(GLenum target, GLuint id)
{
    if (target != GL_TEXTURE_2D)
    {
        sas_error = GL_INVALID_ENUM;
        return;
    }


    if (!id || (id > SAS_TEXTURES) || (sas_textures_2d[id - 1] == NULL))
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    sas_texture_units_2d[sas_current_texture_unit] = id - 1;
}
