#include <stdbool.h>
#include <stddef.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/context.h>
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

SAS_MATRIX_TYPE *sas_current_matrix = sas_modelview;

struct sas_matrix_stack *sas_modelview_stack = NULL, *sas_projection_stack = NULL;
struct sas_matrix_stack **sas_current_matrix_stack = &sas_modelview_stack;

SAS_DEPTH_TYPE sas_far_depth = 1., sas_near_depth = 0.;

int sas_current_mode = -1;

sas_color_t sas_current_color = { .r = 1.f, .g = 1.f, .b = 1.f, .a = 1.f };


bool sas_do_depth_test = false, sas_do_alpha_test = false;

float sas_alpha_ref;

bool (*sas_depth_func)(float new, float current);
bool (*sas_alpha_func)(float new, float ref);


void (*sas_vertex_transformation)(void);
void (*sas_fragment_transformation)(void);


float sas_current_texcoord[8][4], sas_multi_texcoord0[4];
float sas_current_vertex[4], sas_current_position[4];

float sas_triangle_positions[3][4], sas_triangle_texcoords[3][4];
sas_color_t sas_triangle_colors[3];
int sas_triangle_index;

float sas_quad_positions[4][4], sas_quad_texcoords[4][4];
sas_color_t sas_quad_colors[4];
int sas_quad_index;


int sas_current_texture_unit;
bool sas_2d_textures_enabled;
