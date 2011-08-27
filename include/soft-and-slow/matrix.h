#ifndef SAS_MATRIX_H
#define SAS_MATRIX_H

#include "types.h"


struct sas_matrix_stack
{
    struct sas_matrix_stack *next;
    SAS_MATRIX_TYPE *matrix;
};


void sas_multiply_matrix(SAS_MATRIX_TYPE *d, SAS_MATRIX_TYPE *s);
void sas_matrix_dot_vector(SAS_MATRIX_TYPE *matrix, SAS_MATRIX_TYPE *vector);

#endif
