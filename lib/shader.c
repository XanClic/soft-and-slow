#include <dlfcn.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/glext.h>
#include <soft-and-slow/types.h>


// Maybe appropriate. Anyway, easy to adjust.
#define SAS_MAX_PROGS   16
#define SAS_MAX_SHADERS (4 * SAS_MAX_PROGS)


extern GLenum sas_error;

extern void (*sas_vertex_transformation)(void);


struct sas_shader;

struct sas_shader_list
{
    struct sas_shader_list *next;

    struct sas_shader *shader;
};

struct sas_program
{
    void *dl;

    struct sas_shader_list *shaders;
};

struct sas_shader
{
    size_t refcount;

    char *source;

    size_t compiled_len;
    void *compiled;
};

static struct sas_program *programs[SAS_MAX_PROGS];
static struct sas_shader *shaders[SAS_MAX_SHADERS];


// Of course, this function may return 0 though succeeding. But since the init
// function is the first one to call glCreateProgram, the shader emulating the
// fixed pipeline will get that ID, which is simply perfect.
GLuint glCreateProgram(void)
{
    for (int i = 0; i < SAS_MAX_PROGS; i++)
    {
        // FIXME: Lock
        if (programs[i] == NULL)
        {
            programs[i] = malloc(sizeof(*programs[i]));

            programs[i]->shaders = NULL;
            programs[i]->dl = NULL;

            return i;
        }
    }


    sas_error = GL_OUT_OF_MEMORY;

    return 0;
}


// Again, this function may return 0 in case of success, and again this will
// only happen for the vertex shader belonging to the fixed pipeline (that
// shader will never be unloaded, thus index 0 will always be occupied by it).
GLuint glCreateShader(GLenum type)
{
    if (type != GL_VERTEX_SHADER)
    {
        sas_error = GL_INVALID_ENUM;
        return 0;
    }

    for (int i = 0; i < SAS_MAX_PROGS; i++)
    {
        if (shaders[i] == NULL)
        {
            shaders[i] = malloc(sizeof(*shaders[i]));

            shaders[i]->refcount = 1;
            shaders[i]->source = NULL;
            shaders[i]->compiled = NULL;

            return i;
        }
    }


    sas_error = GL_OUT_OF_MEMORY;

    return 0;
}


// Definitions required to turn GLSL into C++
static const char *vtx_shd_def =
   "extern \"C\" void sas_multiply_matrix(float *d, float *s);\n"
   "extern \"C\" void sas_matrix_dot_vector(float *matrix, float *vector);\n"
   "\n"
   "class vec4\n"
   "{\n"
   "    public:\n"
   "        float operator[](int i) { return v[i]; }\n"
   "\n"
   "        union\n"
   "        {\n"
   "            float v[4];\n"
   "            struct { float x, y, z, w; } __attribute__((packed));\n"
   "            struct { float r, g, b, a; } __attribute__((packed));\n"
   "            struct { float s, t, p, q; } __attribute__((packed));\n"
   "        };\n"
   "};\n"
   "\n"
   "class mat4\n"
   "{\n"
   "    public:\n"
   "        float operator[](int i) { return v[i]; }\n"
   "        mat4 operator*(mat4 &m)\n"
   "        {\n"
   "            mat4 ret(*this);\n"
   "            sas_multiply_matrix(ret.v, m.v);\n"
   "            return ret;\n"
   "        }\n"
   "        vec4 operator*(vec4 &vec)\n"
   "        {\n"
   "            vec4 ret(vec);\n"
   "            sas_matrix_dot_vector(v, ret.v);\n"
   "            return ret;\n"
   "        }\n"
   "\n"
   "        float v[16];\n"
   "};\n"
   "\n"
   "#define gl_ModelViewMatrix sas_modelview\n"
   "#define gl_ProjectionMatrix sas_projection\n"
   "#define gl_ModelViewProjectionMatrix sas_modelviewprojection\n"
   "#define gl_Color sas_current_color\n"
   "#define gl_Vertex sas_current_vertex\n"
   "#define gl_Position sas_current_position\n"
   "#define gl_TexCoord sas_current_texcoord\n"
   "#define gl_MultiTexCoord0 sas_multi_texcoord0\n"
   "\n"
   "extern \"C\" vec4 gl_Vertex, gl_Position;\n"
   "extern \"C\" vec4 gl_TexCoord[8], gl_MultiTexCoord0;\n"
   "\n"
   "extern \"C\" mat4 gl_ModelViewProjectionMatrix;\n"
   "extern \"C\" mat4 gl_ModelViewMatrix, gl_ProjectionMatrix;\n"
   "\n"
   "extern \"C\" vec4 gl_Color;\n"
   "\n"
   "static vec4 ftransform(void)\n"
   "{\n"
   "    return gl_ModelViewProjectionMatrix * gl_Vertex;\n"
   "}\n"
   "\n"
   "extern \"C\" void sas_vertex_transform(void);\n";


void glShaderSource(GLuint id, GLsizei count, const GLchar **string, const GLint *length)
{
    if (shaders[id] == NULL)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    free(shaders[id]->source);


    size_t total_length = 0;

    for (int i = 0; i < count; i++)
    {
        size_t this_len;

        if ((length != NULL) && (length[i] >= 0))
            this_len = length[i];
        else
            this_len = strlen(string[i]);

        // Add one character for EOL
        if (this_len)
            this_len++;

        total_length += this_len;
    }


    // One more for NUL and some more for some include
    shaders[id]->source = malloc(total_length + 1 + strlen(vtx_shd_def));

    char *src = shaders[id]->source;
    strcpy(src, vtx_shd_def);
    src += strlen(vtx_shd_def);


    for (int i = 0; i < count; i++)
    {
        size_t this_len;

        if ((length != NULL) && (length[i] >= 0))
            this_len = length[i];
        else
            this_len = strlen(string[i]);


        if (this_len)
        {
            memcpy(src, string[i], this_len);
            src += this_len;
            *(src++) = '\n';
        }
    }

    *src = 0;
}


void glCompileShader(GLuint id)
{
    if (shaders[id] == NULL)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    shaders[id]->compiled_len = 0;
    free(shaders[id]->compiled);


    // FIXME: Not that good when multiple threads are doing this
    FILE *tfp = fopen("/tmp/.soft-and-slow.cpp", "w");
    fputs(shaders[id]->source, tfp);
    fclose(tfp);


    system("sed -i 's/^\\s*#\\s*version.*$//g' /tmp/.soft-and-slow.cpp");
    system("sed -i 's/void\\s\\+main(.*)/void sas_vertex_transform(void)/' /tmp/.soft-and-slow.cpp");

    if (system("g++ -Wall -Wextra -pedantic -std=c++0x -O3 -fPIC -c /tmp/.soft-and-slow.cpp -o /tmp/.soft-and-slow.o"))
    {
        fprintf(stderr, "Compiling failed, leaving patched shader in /tmp/.soft-and-slow.cpp.\n");
        return;
    }


    remove("/tmp/.soft-and-slow.cpp");

    tfp = fopen("/tmp/.soft-and-slow.o", "rb");
    fseek(tfp, 0, SEEK_END);
    shaders[id]->compiled_len = ftell(tfp);
    rewind(tfp);

    shaders[id]->compiled = malloc(shaders[id]->compiled_len);
    fread(shaders[id]->compiled, 1, shaders[id]->compiled_len, tfp);

    fclose(tfp);

    remove("/tmp/.soft-and-slow.o");
}


void glAttachShader(GLuint program, GLuint shader)
{
    if ((programs[program] == NULL) || (shaders[shader] == NULL))
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }

    struct sas_shader_list *nsl = malloc(sizeof(*nsl));
    nsl->shader = shaders[shader];

    shaders[shader]->refcount++;


    // FIXME: Do it atomically
    nsl->next = programs[program]->shaders;
    programs[program]->shaders = nsl;
}

void glLinkProgram(GLuint id)
{
    if (programs[id] == NULL)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    if (programs[id]->dl != NULL)
        dlclose(programs[id]->dl);


    for (struct sas_shader_list *sl = programs[id]->shaders; sl != NULL; sl = sl->next)
    {
        char name[64];
        sprintf(name, "/tmp/.soft-and-slow-%p.o", (void *)sl->shader);

        FILE *tfp = fopen(name, "wb");
        fwrite(sl->shader->compiled, 1, sl->shader->compiled_len, tfp);
        fclose(tfp);
    }

    char soname[64];
    sprintf(soname, "/tmp/.soft-and-slow-%i.so", (int)id);

    char cmd[128];
    sprintf(cmd, "ld -fPIC -shared /tmp/.soft-and-slow-0x*.o -o %s", soname);

    if (system(cmd))
    {
        fprintf(stderr, "Linking failed, leaving object files in /tmp/.soft-and-slow.0x*.o.\n");
        return;
    }

    system("rm /tmp/.soft-and-slow-0x*.o");


    programs[id]->dl = dlopen(soname, RTLD_NOW);

    if (programs[id]->dl == NULL)
    {
        fprintf(stderr, "Could not load shared library (%s).\n", dlerror());
        return;
    }
}

void glDeleteShader(GLuint id)
{
    if (shaders[id] == NULL)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    if (!--shaders[id]->refcount)
    {
        free(shaders[id]->source);
        free(shaders[id]->compiled);
        free(shaders[id]);

        shaders[id] = NULL;
    }
}


void glUseProgram(GLuint id)
{
    sas_vertex_transformation = (void (*)(void))(uintptr_t)dlsym(programs[id]->dl, "sas_vertex_transform");
}
