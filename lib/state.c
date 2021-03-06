#include <stdbool.h>
#include <stddef.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/context.h>
#include <soft-and-slow/light.h>
#include <soft-and-slow/limits.h>
#include <soft-and-slow/matrix.h>
#include <soft-and-slow/types.h>


SAS_COLOR_TYPE sas_clear_color = { .c = 0 };
SAS_DEPTH_TYPE sas_clear_depth = 1.;
SAS_STENCIL_TYPE sas_clear_stencil = 0;

GLenum sas_error = GL_NO_ERROR;

SAS_MATRIX_TYPE sas_modelview[16] = {
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.
};
SAS_MATRIX_TYPE sas_projection[16] = {
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.
};
SAS_MATRIX_TYPE sas_modelviewprojection[16] = {
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.
};
// FIXME FIXME FIXME
SAS_MATRIX_TYPE sas_normal_matrix[9] = {
    1.,  0.,  0.,
    0.,  0.,  1.,
    0., -1.,  0.
};

SAS_MATRIX_TYPE *sas_current_matrix = sas_modelview;

struct sas_matrix_stack *sas_modelview_stack = NULL, *sas_projection_stack = NULL;
struct sas_matrix_stack **sas_current_matrix_stack = &sas_modelview_stack;

SAS_DEPTH_TYPE sas_far_depth = 1., sas_near_depth = 0.;

int sas_current_mode = -1;

sas_color_t sas_current_color = { .r = 1.f, .g = 1.f, .b = 1.f, .a = 1.f };

float sas_current_normal[3];


bool sas_do_depth_test = false, sas_do_alpha_test = false;
// Cull clockwise or counter-clockwise faces, respectively
bool sas_do_cw_culling = false, sas_do_ccw_culling = false;
GLenum sas_cull_face = GL_BACK, sas_front_face = GL_CCW;
bool sas_smooth_shading = true;

float sas_alpha_ref;

bool (*sas_depth_func)(float new, float current);
bool (*sas_alpha_func)(float new, float ref);


void (*sas_vertex_transformation)(void);
void (*sas_fragment_transformation)(void);


float sas_current_texcoord[SAS_TEX_UNITS][4], sas_multi_texcoord0[4];
float sas_current_vertex[4], sas_current_position[4];

unsigned sas_current_buf_index;

float sas_triangle_positions[3][4], sas_triangle_texcoords[3][4];
sas_color_t sas_triangle_colors[3];
int sas_triangle_index;

float sas_quad_positions[4][4], sas_quad_texcoords[4][4];
sas_color_t sas_quad_colors[4];
int sas_quad_index;


int sas_current_texture_unit;
bool sas_2d_textures_enabled;

sas_texture_2d_t *sas_textures_2d[SAS_TEXTURES];
GLuint sas_texture_units_2d[SAS_TEX_UNITS];



bool sas_lighting_enabled = false;

// All are disabled per default, which is how "enabled" is statically
// initialized (to false).
sas_light_t sas_lights[SAS_LIGHTS] = {
    [0] = {
        .position = { 0.f, 0.f, 1.f, 0.f },
        .ambient  = { 0.f, 0.f, 0.f, 1.f },
        .diffuse  = { 1.f, 1.f, 1.f, 1.f },
        .specular = { 1.f, 1.f, 1.f, 1.f }
    },
    [1] = {
        .position = { 0.f, 0.f, 1.f, 0.f },
        .ambient  = { 0.f, 0.f, 0.f, 1.f },
        .diffuse  = { 0.f, 0.f, 0.f, 1.f },
        .specular = { 0.f, 0.f, 0.f, 1.f }
    },
    [2] = {
        .position = { 0.f, 0.f, 1.f, 0.f },
        .ambient  = { 0.f, 0.f, 0.f, 1.f },
        .diffuse  = { 0.f, 0.f, 0.f, 1.f },
        .specular = { 0.f, 0.f, 0.f, 1.f }
    },
    [3] = {
        .position = { 0.f, 0.f, 1.f, 0.f },
        .ambient  = { 0.f, 0.f, 0.f, 1.f },
        .diffuse  = { 0.f, 0.f, 0.f, 1.f },
        .specular = { 0.f, 0.f, 0.f, 1.f }
    },
    [4] = {
        .position = { 0.f, 0.f, 1.f, 0.f },
        .ambient  = { 0.f, 0.f, 0.f, 1.f },
        .diffuse  = { 0.f, 0.f, 0.f, 1.f },
        .specular = { 0.f, 0.f, 0.f, 1.f }
    },
    [5] = {
        .position = { 0.f, 0.f, 1.f, 0.f },
        .ambient  = { 0.f, 0.f, 0.f, 1.f },
        .diffuse  = { 0.f, 0.f, 0.f, 1.f },
        .specular = { 0.f, 0.f, 0.f, 1.f }
    },
    [6] = {
        .position = { 0.f, 0.f, 1.f, 0.f },
        .ambient  = { 0.f, 0.f, 0.f, 1.f },
        .diffuse  = { 0.f, 0.f, 0.f, 1.f },
        .specular = { 0.f, 0.f, 0.f, 1.f }
    },
    [7] = {
        .position = { 0.f, 0.f, 1.f, 0.f },
        .ambient  = { 0.f, 0.f, 0.f, 1.f },
        .diffuse  = { 0.f, 0.f, 0.f, 1.f },
        .specular = { 0.f, 0.f, 0.f, 1.f }
    }
};

sas_material_t sas_current_material = {
    .ambient  = { .2f, .2f, .2f, 1.f },
    .diffuse  = { .8f, .8f, .8f, 1.f },
    .specular = { 0.f, 0.f, 0.f, 1.f },
    .emission = { 0.f, 0.f, 0.f, 1.f },

    .shininess = 0.f
};

bool sas_normalize_normals = false;


sas_command_list_t **sas_current_command_list;
// Execute current command list (while expanding)
bool sas_execute_ccl;
