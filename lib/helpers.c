#include <stddef.h>
#include <string.h>

#include <soft-and-slow/helpers.h>

void memset_color(SAS_COLOR_TYPE *buf, SAS_COLOR_TYPE value, size_t length)
{
    while (length--)
        *(buf++) = value;
}

void memset_depth(SAS_DEPTH_TYPE *buf, SAS_DEPTH_TYPE value, size_t length)
{
    while (length--)
        *(buf++) = value;
}

void memset_stencil(SAS_STENCIL_TYPE *buf, SAS_STENCIL_TYPE value, size_t length)
{
    if (sizeof(SAS_STENCIL_TYPE) == sizeof(uint8_t))
        memset(buf, value, length);
    else
    {
        while (length--)
            *(buf++) = value;
    }
}
