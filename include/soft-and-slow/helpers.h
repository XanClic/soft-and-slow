#ifndef SAS_HELPERS_H
#define SAS_HELPERS_H

#include <stddef.h>

#include "context.h"
#include "types.h"


#define clampf(v) (((v) >= 1.f) ? 1.f : (((v) <= 0.f) ? 0.f : (v)))
#define clampd(v) (((v) >= 1. ) ? 1.  : (((v) <= 0. ) ? 0.  : (v)))


// Fills the first <length> pixels of <buf> with <value>
void memset_color  (SAS_COLOR_TYPE   *buf, SAS_COLOR_TYPE   value, size_t length);
void memset_depth  (SAS_DEPTH_TYPE   *buf, SAS_DEPTH_TYPE   value, size_t length);
void memset_stencil(SAS_STENCIL_TYPE *buf, SAS_STENCIL_TYPE value, size_t length);

#endif
