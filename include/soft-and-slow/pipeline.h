#ifndef SAS_PIPELINE_H
#define SAS_PIPELINE_H

#include <stdbool.h>

#include "context.h"
#include "matrix.h"
#include "types.h"


bool sas_transform_vertex_to_screen(sas_context_t ctx, const SAS_MATRIX_TYPE *vec, sas_wnd_coord_t *wc);
bool sas_depth_test(sas_context_t ctx, sas_wnd_coord_t wc);
bool sas_alpha_test(float alpha);
void sas_blend_pixel(sas_context_t ctx, sas_wnd_coord_t wc, sas_color_t color);

#endif
