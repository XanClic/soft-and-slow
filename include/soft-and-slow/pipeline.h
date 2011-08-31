#ifndef SAS_PIPELINE_H
#define SAS_PIPELINE_H

#include <stdbool.h>

#include "context.h"
#include "matrix.h"
#include "types.h"


void sas_transform_vertex_to_screen(void);
void sas_transform_fragment(void);

#endif
