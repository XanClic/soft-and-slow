#ifndef SAS_PIPELINE_H
#define SAS_PIPELINE_H

#include <stdbool.h>

#include "context.h"
#include "matrix.h"
#include "types.h"


bool sas_transform_vertex_to_screen(void);
void sas_transform_fragment(void);

bool sas_depth_test(sas_context_t ctx, unsigned x, unsigned y, float d);
bool sas_alpha_test(float alpha);
void sas_blend_pixel(sas_context_t ctx, unsigned x, unsigned y, sas_color_t color);

#endif
