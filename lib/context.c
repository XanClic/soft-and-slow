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
    if (!init_called)
    {
        sas_init();
        init_called = true;
    }


    sas_context_t ctx = malloc(sizeof(*ctx));

    ctx->width = width;
    ctx->height = height;

    ctx->colorbuffer = malloc(width * height * sizeof(SAS_COLOR_TYPE));
    ctx->depthbuffer = malloc(width * height * sizeof(SAS_DEPTH_TYPE));
    ctx->stencilbuffer = malloc(width * height * sizeof(SAS_STENCIL_TYPE));


    ctx->__checkbuffer = malloc(width * height);


    return ctx;
}


void destroy_sas_context(sas_context_t ctx)
{
    free(ctx->colorbuffer);
    free(ctx->depthbuffer);
    free(ctx->stencilbuffer);

    free(ctx->__checkbuffer);

    free(ctx);
}


void set_current_sas_context(sas_context_t ctx)
{
    current_sas_context = ctx;
}
