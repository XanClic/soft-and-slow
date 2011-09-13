#ifndef SAS_CONTEXT_H
#define SAS_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

#include "compiler.h"
#include "types.h"


#define SAS_STENCIL_BITS (sizeof(SAS_STENCIL_TYPE) * 8)


typedef struct sas_context *sas_context_t;

// Contains all necessary data belonging to an S&S context
struct sas_context
{
    // Width and height, obviously
    unsigned width, height;
    // Color data
    SAS_COLOR_TYPE *colorbuffer;
    // Depth information
    SAS_DEPTH_TYPE *depthbuffer;
    // Stencil data
    SAS_STENCIL_TYPE *stencilbuffer;

    // True, if the color buffer has been allocated by create_sas_context
    bool allocated_colorbuffer;
};


// Creates a new S&S context with the given width and heigth
sas_context_t create_sas_context(unsigned width, unsigned height);
// Also creates a new S&S context, but uses the given memory area as the color
// buffer.
sas_context_t create_bound_sas_context(unsigned width, unsigned height, void *colorbuffer);

// Destroys the given S&S context
void destroy_sas_context(sas_context_t context);

// Sets the current S&S context
void set_current_sas_context(sas_context_t context);


#ifdef __cplusplus
}
#endif

#endif
