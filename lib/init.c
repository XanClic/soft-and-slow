#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/glext.h>


extern char _binary_fixed_vertex_transformation_glsl_start[];
extern const void _binary_fixed_vertex_transformation_glsl_size;

extern char _binary_fixed_fragment_transformation_glsl_start[];
extern const void _binary_fixed_fragment_transformation_glsl_size;

void sas_init(void)
{
    glAlphaFunc(GL_ALWAYS, .0f);

    glDepthFunc(GL_LESS);


    GLuint sas_fixed_pipeline = glCreateProgram();
    assert(!sas_fixed_pipeline);


    GLuint fixed_vertex = glCreateShader(GL_VERTEX_SHADER);

    char *src = _binary_fixed_vertex_transformation_glsl_start;
    int sz = (uintptr_t)&_binary_fixed_vertex_transformation_glsl_size;
    glShaderSource(fixed_vertex, 1, (const GLchar **)&src, &sz);
    glCompileShader(fixed_vertex);

    glAttachShader(sas_fixed_pipeline, fixed_vertex);
    glDeleteShader(fixed_vertex);


    GLuint fixed_fragment = glCreateShader(GL_FRAGMENT_SHADER);

    src = _binary_fixed_fragment_transformation_glsl_start;
    sz = (uintptr_t)&_binary_fixed_fragment_transformation_glsl_size;
    glShaderSource(fixed_fragment, 1, (const GLchar **)&src, &sz);
    glCompileShader(fixed_fragment);

    glAttachShader(sas_fixed_pipeline, fixed_fragment);
    glDeleteShader(fixed_fragment);


    glLinkProgram(sas_fixed_pipeline);


    // The assert above made sure that sas_fixed_pipeline == 0
    glUseProgram(0);


    glActiveTexture(GL_TEXTURE0);
}
