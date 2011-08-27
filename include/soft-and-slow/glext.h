#ifndef SAS_GLEXT_H
#define SAS_GLEXT_H

#include "types.h"


GLuint glCreateProgram(void);
void glLinkProgram(GLuint id);
void glUseProgram(GLuint id);

GLuint glCreateShader(GLenum type);
void glShaderSource(GLuint id, GLsizei count, const GLchar **string, const GLint *length);
void glCompileShader(GLuint id);
void glAttachShader(GLuint program, GLuint shader);
void glDeleteShader(GLuint id);

GLint glGetUniformLocation(GLuint program, const GLchar *name);
void glUniform1i(GLint location, GLint v0);

#endif
