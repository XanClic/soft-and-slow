#ifndef SAS_LIMITS_H
#define SAS_LIMITS_H

// Maybe appropriate. Anyway, easy to adjust.

// Number of texture units
#define SAS_TEX_UNITS 8
// Number of textures per type (e. g., GL_TEXTURE_2D)
#define SAS_TEXTURES 128


// Number of programs
#define SAS_MAX_PROGS   16
// Number of shaders
#define SAS_MAX_SHADERS (4 * SAS_MAX_PROGS)

// Number of uniforms per program
#define SAS_MAX_UNIFORMS 32

// Number of lights
#define SAS_LIGHTS 8

// Maximum number of display lists
#define SAS_MAX_DISPLAY_LISTS 32

#endif
