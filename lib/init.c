#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/glext.h>
#include <soft-and-slow/threads.h>


extern char _binary_fixed_vertex_transformation_glsl_start[];
extern const void _binary_fixed_vertex_transformation_glsl_size;

extern char _binary_fixed_fragment_transformation_glsl_start[];
extern const void _binary_fixed_fragment_transformation_glsl_size;

extern char _binary_fixed_vertex_light_transformation_glsl_start[];
extern const void _binary_fixed_vertex_light_transformation_glsl_size;

extern char _binary_fixed_fragment_light_transformation_glsl_start[];
extern const void _binary_fixed_fragment_light_transformation_glsl_size;


void sas_load_fixed_pipeline(bool lighting)
{
    // This is not exactly according to the specification (it actually opposes
    // it), but who cares anyway?
    glDeleteProgram(0);


    assert(!glCreateProgram());


    GLuint fixed_vertex = glCreateShader(GL_VERTEX_SHADER);

    char *src = lighting ?
        _binary_fixed_vertex_light_transformation_glsl_start :
        _binary_fixed_vertex_transformation_glsl_start;

    int sz = lighting ?
        (uintptr_t)&_binary_fixed_vertex_light_transformation_glsl_size :
        (uintptr_t)&_binary_fixed_vertex_transformation_glsl_size;

    glShaderSource(fixed_vertex, 1, (const GLchar **)&src, &sz);
    glCompileShader(fixed_vertex);


    GLint illen;
    glGetShaderiv(fixed_vertex, GL_INFO_LOG_LENGTH, &illen);
    if (illen > 1)
    {
        char *msg = malloc(illen);
        glGetShaderInfoLog(fixed_vertex, illen, NULL, msg);

        printf("%s", msg);

        free(msg);
    }

    GLint status;
    glGetShaderiv(fixed_vertex, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        fprintf(stderr, "Could not compile vertex shader.\n");
        abort();
    }


    glAttachShader(0, fixed_vertex);
    glDeleteShader(fixed_vertex);


    GLuint fixed_fragment = glCreateShader(GL_FRAGMENT_SHADER);

    src = lighting ?
        _binary_fixed_fragment_light_transformation_glsl_start :
        _binary_fixed_fragment_transformation_glsl_start;

    sz = lighting ?
        (uintptr_t)&_binary_fixed_fragment_light_transformation_glsl_size :
        (uintptr_t)&_binary_fixed_fragment_transformation_glsl_size;

    glShaderSource(fixed_fragment, 1, (const GLchar **)&src, &sz);
    glCompileShader(fixed_fragment);


    glGetShaderiv(fixed_vertex, GL_INFO_LOG_LENGTH, &illen);
    if (illen > 1)
    {
        char *msg = malloc(illen);
        glGetShaderInfoLog(fixed_fragment, illen, NULL, msg);

        printf("%s", msg);

        free(msg);
    }

    glGetShaderiv(fixed_fragment, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        fprintf(stderr, "Could not compile fragment shader.\n");
        abort();
    }

    glAttachShader(0, fixed_fragment);
    glDeleteShader(fixed_fragment);


    glLinkProgram(0);


    glUseProgram(0);
}


void sas_init(void)
{
    glAlphaFunc(GL_ALWAYS, .0f);

    glDepthFunc(GL_LESS);


    sas_load_fixed_pipeline(false);


    glActiveTexture(GL_TEXTURE0);


#ifdef THREADING
    sas_spawn_threads();
#endif
}
