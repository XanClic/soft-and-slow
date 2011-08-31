#if defined __glu_h__ || defined __glu_h_ || defined __glu_h || defined glu_h || defined _glu_h || defined _glu_h_
#warning "Won't using soft-and-slow/glu.h, since GL/glu.h has already been included."
#else
#ifndef SAS_GLU_H
#define SAS_GLU_H
#define __glu_h__
#define __glu_h_
#define __glu_h
#define  _glu_h_
#define  _glu_h
#define   glu_h

#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"


void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble znear, GLdouble zfar);


#ifdef __cplusplus
}
#endif

#endif
#endif
