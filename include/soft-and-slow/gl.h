#if defined __gl_h__ || defined __gl_h_ || defined __gl_h || defined gl_h || defined _gl_h || defined _gl_h_
#warning "Won't using soft-and-slow/gl.h, since GL/gl.h has already been included."
#else
#ifndef SAS_GL_H
#define SAS_GL_H
#define __gl_h__
#define __gl_h_
#define __gl_h
#define  _gl_h_
#define  _gl_h
#define   gl_h

#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"

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
GLenum glGetError(void);
void glCullFace(GLenum mode);
void glFrontFace(GLenum mode);

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

void glActiveTexture(GLenum unit);
void glBindTexture(GLenum target, GLuint id);
void glGenTextures(GLsizei n, GLuint *textures);
void glTexImage2D(GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *data);
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *data);

void glLightfv(GLenum light, GLenum pname, const GLfloat *params);
void glMaterialf(GLenum face, GLenum pname, const GLfloat param);
void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
void glNormal3f(GLfloat x, GLfloat y, GLfloat z);


#ifdef __cplusplus
}
#endif

#endif
#endif
