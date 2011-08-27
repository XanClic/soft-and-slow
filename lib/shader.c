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

#define SAS_MAX_UNIFORMS 32


extern GLenum sas_error;

extern void (*sas_vertex_transformation)(void);
extern void (*sas_fragment_transformation)(void);


// Definitions required to turn GLSL into C++
extern char _binary_vertex_shader_include_hpp_start[], _binary_fragment_shader_include_hpp_start[];
extern const void _binary_vertex_shader_include_hpp_size, _binary_fragment_shader_include_hpp_size;


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

    void *uniforms[SAS_MAX_UNIFORMS];
};

struct uniform_list
{
    struct uniform_list *next;

    void *address;
};

struct sas_shader
{
    size_t refcount;

    GLenum type;

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
    if ((type != GL_VERTEX_SHADER) && (type != GL_FRAGMENT_SHADER))
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

            shaders[i]->type = type;
            shaders[i]->source = NULL;
            shaders[i]->compiled = NULL;

            return i;
        }
    }


    sas_error = GL_OUT_OF_MEMORY;

    return 0;
}


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
    if (shaders[id]->type == GL_VERTEX_SHADER)
        shaders[id]->source = malloc(total_length + 1 + (uintptr_t)&_binary_vertex_shader_include_hpp_size);
    else
        shaders[id]->source = malloc(total_length + 1 + (uintptr_t)&_binary_fragment_shader_include_hpp_size);

    char *src = shaders[id]->source;
    if (shaders[id]->type == GL_VERTEX_SHADER)
    {
        memcpy(src, _binary_vertex_shader_include_hpp_start, (uintptr_t)&_binary_vertex_shader_include_hpp_size);
        src += (uintptr_t)&_binary_vertex_shader_include_hpp_size;
    }
    else
    {
        memcpy(src, _binary_fragment_shader_include_hpp_start, (uintptr_t)&_binary_fragment_shader_include_hpp_size);
        src += (uintptr_t)&_binary_fragment_shader_include_hpp_size;
    }


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

    if (shaders[id]->type == GL_VERTEX_SHADER)
        system("sed -i 's/void\\s\\+main(.*)/void sas_vertex_transform(void)/' /tmp/.soft-and-slow.cpp");
    else
        system("sed -i 's/void\\s\\+main(.*)/void sas_fragment_transform(void)/' /tmp/.soft-and-slow.cpp");

    system("sed -i 's/uniform\\s\\+\\(\\w\\+\\)\\s\\+\\(\\w\\+\\)/#define \\2 sas_uniform_\\2\\n\\1 \\2/g' /tmp/.soft-and-slow.cpp");

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


    for (int i = 0; i < SAS_MAX_UNIFORMS; i++)
        programs[id]->uniforms[i] = NULL;


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
    sas_vertex_transformation   = (void (*)(void))(uintptr_t)dlsym(programs[id]->dl, "sas_vertex_transform");
    sas_fragment_transformation = (void (*)(void))(uintptr_t)dlsym(programs[id]->dl, "sas_fragment_transform");
}


GLint glGetUniformLocation(GLuint id, const GLchar *name)
{
    if (programs[id] == NULL)
    {
        sas_error = GL_INVALID_VALUE;
        return -1;
    }


    char tname[strlen(name) + 32];
    sprintf(tname, "sas_uniform_%s", name);

    void *ptr = dlsym(programs[id]->dl, tname);


    for (int i = 0; i < SAS_MAX_UNIFORMS; i++)
        if (programs[id]->uniforms[i] == ptr)
            return id * SAS_MAX_UNIFORMS + i;

    for (int i = 0; i < SAS_MAX_UNIFORMS; i++)
    {
        // FIXME: Atomically
        if (programs[id]->uniforms[i] == NULL)
        {
            programs[id]->uniforms[i] = ptr;
            return id * SAS_MAX_UNIFORMS + i;
        }
    }


    sas_error = GL_OUT_OF_MEMORY;

    return -1;
}

void glUniform1i(GLint location, GLint v0)
{
    GLuint prog = location / SAS_MAX_UNIFORMS;
    location %= SAS_MAX_UNIFORMS;

    if ((programs[prog] == NULL) || (programs[prog]->uniforms[location] == NULL))
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    *(GLint *)programs[prog]->uniforms[location] = v0;
}
