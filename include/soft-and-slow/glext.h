#ifndef SAS_GLEXT_H
#define SAS_GLEXT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"


GLuint glCreateProgram(void);
void glLinkProgram(GLuint id);
void glUseProgram(GLuint id);
void glDeleteProgram(GLuint id);

GLuint glCreateShader(GLenum type);
void glShaderSource(GLuint id, GLsizei count, const GLchar **string, const GLint *length);
void glCompileShader(GLuint id);
void glAttachShader(GLuint program, GLuint shader);
void glDeleteShader(GLuint id);

GLint glGetUniformLocation(GLuint program, const GLchar *name);
void glUniform1i(GLint location, GLint v0);
void glUniform1f(GLint location, GLfloat v0);
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);

void glGetShaderiv(GLuint id, GLenum pname, GLint *params);
void glGetShaderInfoLog(GLuint id, GLsizei max_length, GLsizei *length, GLchar *log);


#ifdef __cplusplus
}
#endif

#endif
