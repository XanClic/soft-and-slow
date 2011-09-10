#ifndef SAS_COMMAND_LISTS_H
#define SAS_COMMAND_LISTS_H

#include "types.h"


enum sas_commands
{
    SAS_COMMAND_BEGIN,
    SAS_COMMAND_END,

    SAS_COMMAND_VERTEX,
    SAS_COMMAND_COLOR,
    SAS_COMMAND_TEXCOORD,

    SAS_COMMAND_NORMAL,

    SAS_COMMAND_MATERIAL_S,
    SAS_COMMAND_MATERIAL_V,

    SAS_COMMAND_SHADE_MODEL
};

typedef struct sas_command_list sas_command_list_t;

struct sas_command_list
{
    sas_command_list_t *next;

    enum sas_commands cmd;
};

struct sas_command_begin
{
    sas_command_list_t cl;

    GLenum mode;
};

struct sas_command_vertex
{
    sas_command_list_t cl;

    float position[4];
};

struct sas_command_color
{
    sas_command_list_t cl;

    sas_color_t color;
};

struct sas_command_texcoord
{
    sas_command_list_t cl;

    float coord[4];
};

struct sas_command_normal
{
    sas_command_list_t cl;

    float normal[3];
};

struct sas_command_material_s
{
    sas_command_list_t cl;

    GLenum face;
    GLenum material_cmd;
    float param;
};

struct sas_command_material_v
{
    sas_command_list_t cl;

    GLenum face;
    GLenum material_cmd;
    float *param;
};

struct sas_command_shade_model
{
    sas_command_list_t cl;

    GLenum model;
};


#endif
