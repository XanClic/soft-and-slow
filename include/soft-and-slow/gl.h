#ifndef SAS_GL_H
#define SAS_GL_H

#include "constants.h"
#include "types.h"


void glClear(GLbitfield mask);
void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
void glClearDepth(GLclampd d);
void glClearStencil(GLint s);

void glEnable(GLenum cap);
void glDisable(GLenum cap);
GLboolean glIsEnabled(GLenum cap);
void glGetFloatv(GLenum pname, float *params);

void glDepthFunc(GLenum func);
void glAlphaFunc(GLenum func, GLclampf ref);

void glMatrixMode(GLenum mode);
void glLoadIdentity(void);
void glLoadMatrixf(GLfloat *m);
void glLoadMatrixd(GLdouble *m);
void glPushMatrix(void);
void glPopMatrix(void);
void glMultMatrixf(GLfloat *m);
void glMultMatrixd(GLdouble *m);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glTranslated(GLdouble x, GLdouble y, GLdouble z);
void glScalef(GLfloat x, GLfloat y, GLfloat z);
void glScaled(GLdouble x, GLdouble y, GLdouble z);

void glBegin(GLenum mode);
void glEnd(void);

void glColor3f(GLfloat r, GLfloat g, GLfloat b);
void glTexCoord2f(GLfloat s, GLfloat t);
void glVertex3f(GLfloat x, GLfloat y, GLfloat z);

#endif
