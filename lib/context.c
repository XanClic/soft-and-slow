#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <soft-and-slow/context.h>
#include <soft-and-slow/types.h>


sas_context_t current_sas_context;

static bool init_called = false;
extern void sas_init(void);


sas_context_t create_sas_context(unsigned width, unsigned height)
{
    sas_context_t ctx = create_bound_sas_context(width, height, malloc(width * height * sizeof(SAS_COLOR_TYPE)));

    ctx->allocated_colorbuffer = true;

    return ctx;
}

sas_context_t create_bound_sas_context(unsigned width, unsigned height, void *colorbuffer)
{
    if (!init_called)
    {
        sas_init();
        init_called = true;
    }


    sas_context_t ctx = malloc(sizeof(*ctx));

    ctx->width = width;
    ctx->height = height;

    ctx->depthbuffer = malloc(width * height * sizeof(SAS_DEPTH_TYPE));
    ctx->stencilbuffer = malloc(width * height * sizeof(SAS_STENCIL_TYPE));
    ctx->colorbuffer = colorbuffer;

    ctx->allocated_colorbuffer = false;


    return ctx;
}


void destroy_sas_context(sas_context_t ctx)
{
    if (ctx->allocated_colorbuffer)
        free(ctx->colorbuffer);

    free(ctx->depthbuffer);
    free(ctx->stencilbuffer);

    free(ctx);
}


void set_current_sas_context(sas_context_t ctx)
{
    current_sas_context = ctx;
}
