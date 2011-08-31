#ifndef SAS_MATRIX_H
#define SAS_MATRIX_H

#include "types.h"


struct sas_matrix_stack
{
    struct sas_matrix_stack *next;
    SAS_MATRIX_TYPE *matrix;
};


void sas_multiply_matrix(SAS_MATRIX_TYPE *d, const SAS_MATRIX_TYPE *s);
void sas_matrix_dot_vector(const SAS_MATRIX_TYPE *matrix, SAS_MATRIX_TYPE *vector);
void sas_multiply_matrix_3x3(SAS_MATRIX_TYPE *d, const SAS_MATRIX_TYPE *s);
void sas_matrix_dot_vector_3x3(const SAS_MATRIX_TYPE *matrix, SAS_MATRIX_TYPE *vector);

#endif
