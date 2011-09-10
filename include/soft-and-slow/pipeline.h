#ifndef SAS_PIPELINE_H
#define SAS_PIPELINE_H

#include <stdbool.h>

#include "context.h"
#include "matrix.h"
#include "types.h"


void sas_transform_vertex_to_screen(void);

#ifdef THREADING
void sas_transform_fragment(sas_draw_thread_info_t *dti);
#else
void sas_transform_fragment(void);
#endif

#endif
