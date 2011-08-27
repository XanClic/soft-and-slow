#include <assert.h>
#include <stddef.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/glext.h>


extern GLuint sas_fixed_pipeline;

char *fixed_vertex_source = "#version 110\n"
                            "#include <stdio.h>\n"
                            "\n"
                            "void main()\n"
                            "{\n"
                            "    gl_Position = ftransform();\n"
                            "    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
                            "}";


void sas_init(void)
{
    glAlphaFunc(GL_ALWAYS, .0f);

    glDepthFunc(GL_LESS);


    sas_fixed_pipeline = glCreateProgram();
    assert(!sas_fixed_pipeline);


    GLuint fixed_vertex = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(fixed_vertex, 1, (const GLchar **)&fixed_vertex_source, NULL);
    glCompileShader(fixed_vertex);

    glAttachShader(sas_fixed_pipeline, fixed_vertex);
    glDeleteShader(fixed_vertex);


    glLinkProgram(sas_fixed_pipeline);


    // The assert above made sure that sas_fixed_pipeline == 0
    glUseProgram(0);
}
