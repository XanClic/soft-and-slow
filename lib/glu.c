#include <math.h>

#include <soft-and-slow/glu.h>
#include <soft-and-slow/matrix.h>
#include <soft-and-slow/types.h>


extern SAS_MATRIX_TYPE *sas_current_matrix;


void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble znear, GLdouble zfar)
{
    /*
    double cot_fov = 1. / tan(fovy / 2. * M_PI / 180.);

    double persp[16] = {
        cot_fov / aspect,      0.,                              0.,                                  0.,
                      0., cot_fov,                              0.,                                  0.,
                      0.,      0., (znear + zfar) / (znear - zfar), (2 * znear * zfar) / (znear - zfar),
                      0.,      0.,                             -1.,                                  0.
    };

    sas_multiplay_matrix(persp);
    */

    // Inlined
    double c = 1. / tan(fovy / 2. * M_PI / 180.);
    double ca = c / aspect;
    double z1 =    (znear + zfar) / (znear - zfar);
    double z2 = 2 * znear * zfar  / (znear - zfar);

    SAS_MATRIX_TYPE a, b;

    sas_current_matrix[ 0] *= ca; sas_current_matrix[ 1] *= c; a = sas_current_matrix[ 2]; b = sas_current_matrix[ 3]; sas_current_matrix[ 2] = a * z1 - b; sas_current_matrix[ 3] = a * z2;
    sas_current_matrix[ 4] *= ca; sas_current_matrix[ 5] *= c; a = sas_current_matrix[ 6]; b = sas_current_matrix[ 7]; sas_current_matrix[ 6] = a * z1 - b; sas_current_matrix[ 7] = a * z2;
    sas_current_matrix[ 8] *= ca; sas_current_matrix[ 9] *= c; a = sas_current_matrix[10]; b = sas_current_matrix[11]; sas_current_matrix[10] = a * z1 - b; sas_current_matrix[11] = a * z2;
    sas_current_matrix[12] *= ca; sas_current_matrix[13] *= c; a = sas_current_matrix[14]; b = sas_current_matrix[15]; sas_current_matrix[14] = a * z1 - b; sas_current_matrix[15] = a * z2;
}
