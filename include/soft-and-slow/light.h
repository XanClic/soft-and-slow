#ifndef SAS_LIGHT_H
#define SAS_LIGHT_H

#include <stdbool.h>

#include "types.h"


typedef struct sas_light sas_light_t;

struct sas_light
{
    bool enabled;

    float position[4];
    sas_color_t ambient, diffuse, specular;
};


typedef struct sas_material sas_material_t;

struct sas_material
{
    sas_color_t ambient, diffuse, specular, emission;
    float shininess;
};

#endif
