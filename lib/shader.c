#include <assert.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <soft-and-slow/constants.h>
#include <soft-and-slow/glext.h>
#include <soft-and-slow/limits.h>
#include <soft-and-slow/shader.h>
#include <soft-and-slow/types.h>


extern GLenum sas_error;

extern void (*sas_vertex_transformation)(void);
extern void (*sas_fragment_transformation)(void);


// Definitions required to turn GLSL into C++
extern char _binary_vertex_shader_include_hpp_start[], _binary_fragment_shader_include_hpp_start[];
extern char _binary_general_shader_include_hpp_start[], _binary_general_shader_common_cpp_start[];
extern const void _binary_vertex_shader_include_hpp_size, _binary_fragment_shader_include_hpp_size;
extern const void _binary_general_shader_include_hpp_size, _binary_general_shader_common_cpp_size;


struct sas_shader;

struct sas_shader_list
{
    struct sas_shader_list *next;

    struct sas_shader *shader;
};

struct shader_varyings
{
    struct shader_varyings *next;

    char *identifier;

    enum sas_varying_types type;
    size_t size;
};

struct program_varyings
{
    struct program_varyings *next;

    union
    {
        // C++ bool == uint8_t, isn't it? (FIXME if necessary)
        uint8_t *val_bool;
        int *val_int;
        float *val_float;
        sas_xmm_t *val_xmm;

        void *address;
    };

    // TODO: structures
    enum sas_varying_types type;
    // Number of fields (if array, else 1)
    size_t size;


    union
    {
        uint8_t *saved_bool;
        int *saved_int;
        float *saved_float;
        sas_xmm_t *saved_xmm;

        void *saved_values;
    };
};

struct sas_program
{
    void *dl;

    struct sas_shader_list *shaders;

    void *uniforms[SAS_MAX_UNIFORMS];

    struct program_varyings *varyings;
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

    size_t log_len;
    char *log;

    struct shader_varyings *varyings;
};

static struct sas_program *programs[SAS_MAX_PROGS];
static struct sas_shader *shaders[SAS_MAX_SHADERS];

static struct sas_program *current_program = NULL;

static const size_t varying_type_sizes[] = { sizeof(uint8_t), sizeof(int), sizeof(float), sizeof(sas_xmm_t) };
static int varying_index = 0;


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
            programs[i]->varyings = NULL;

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
            shaders[i]->compiled_len = 0;
            shaders[i]->compiled = NULL;
            shaders[i]->log_len = 0;
            shaders[i]->log = NULL;
            shaders[i]->varyings = NULL;

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


    // One more for NUL
    shaders[id]->source = malloc(total_length + 1);

    char *src = shaders[id]->source;

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
    shaders[id]->compiled = NULL;

    shaders[id]->log_len = 0;
    free(shaders[id]->log);
    shaders[id]->log = NULL;

    struct shader_varyings *v = shaders[id]->varyings;
    while (v != NULL)
    {
        struct shader_varyings *vn = v->next;
        free(v->identifier);
        free(v);
        v = vn;
    }
    shaders[id]->varyings = NULL;


    // FIXME: Not that good when multiple threads are doing this
    FILE *tfp = fopen("/tmp/.soft-and-slow.cpp", "w");
    fputs("#include \".soft-and-slow-general.hpp\"\n", tfp);
    fputs("#include \".soft-and-slow-specific.hpp\"\n\n", tfp);
    fputs(shaders[id]->source, tfp);
    fclose(tfp);

    tfp = fopen("/tmp/.soft-and-slow-general.hpp", "w");
    fwrite(&_binary_general_shader_include_hpp_start, 1, (size_t)&_binary_general_shader_include_hpp_size, tfp);
    fclose(tfp);

    tfp = fopen("/tmp/.soft-and-slow-specific.hpp", "w");
    if (shaders[id]->type == GL_VERTEX_SHADER)
        fwrite(&_binary_vertex_shader_include_hpp_start, 1, (size_t)&_binary_vertex_shader_include_hpp_size, tfp);
    else
        fwrite(&_binary_fragment_shader_include_hpp_start, 1, (size_t)&_binary_fragment_shader_include_hpp_size, tfp);
    fclose(tfp);


    system("sed -i 's/^\\s*#\\s*version.*$//g' /tmp/.soft-and-slow.cpp");

    if (shaders[id]->type == GL_VERTEX_SHADER)
        system("sed -i 's/void\\s\\+main(.*)/void sas_vertex_transform(void)/' /tmp/.soft-and-slow.cpp");
    else
        system("sed -i 's/void\\s\\+main(.*)/void sas_fragment_transform(void)/' /tmp/.soft-and-slow.cpp");

    system("sed -i 's/uniform\\s\\+\\(\\w\\+\\)\\s\\+\\(\\w\\+\\)/#define \\2 sas_uniform_\\2\\n\\1 \\2/g' /tmp/.soft-and-slow.cpp");

    if (shaders[id]->type == GL_FRAGMENT_SHADER)
        system("sed -i 's/varying\\s\\+\\(\\w\\+\\)\\s\\+\\(\\w\\+\\)/#define \\2 sas_varying_\\2\\nextern \\1 \\2/g' /tmp/.soft-and-slow.cpp");
    else
    {
        system("grep -e varying /tmp/.soft-and-slow.cpp | sed -e 's/varying\\s\\+\\(\\w\\+\\)\\s\\+\\(\\w\\+\\);/\\1\\n\\2/g' > /tmp/.soft-and-slow.varyings");

        system("sed -i 's/varying\\s\\+\\(\\w\\+\\)\\s\\+\\(\\w\\+\\)/#define \\2 sas_varying_\\2\\n\\1 \\2/g' /tmp/.soft-and-slow.cpp");

        tfp = fopen("/tmp/.soft-and-slow.varyings", "r");
        char line[256];

        struct shader_varyings **lvp = &shaders[id]->varyings;
        while (!feof(tfp) && !ferror(tfp))
        {
            if (fgets(line, 256, tfp) == NULL)
                break;

            if (line[strlen(line) - 1] == '\n')
                line[strlen(line) - 1] = 0;

            struct shader_varyings *v = calloc(1, sizeof(*v));

            if (!strcmp(line, "float")) { v->type = SAS_FLOAT; v->size = 1; }
            else if (!strcmp(line, "int")) { v->type = SAS_INT; v->size = 1; }
            else if (!strcmp(line, "bool")) { v->type = SAS_BOOL; v->size = 1; }
            // Just hoping gcc actually uses 16 bytes to store those classes.
            // I tried to add padding floats, but all it did was segfaulting or
            // doing the wrong thing™.
            else if (!strcmp(line, "vec2")) { v->type = SAS_XMM; v->size = 1; }
            else if (!strcmp(line, "vec3")) { v->type = SAS_XMM; v->size = 1; }
            else if (!strcmp(line, "vec4")) { v->type = SAS_XMM; v->size = 1; }
            else if (!strcmp(line, "vec2i")) { v->type = SAS_INT; v->size = 2; }
            else if (!strcmp(line, "vec3i")) { v->type = SAS_INT; v->size = 3; }
            else if (!strcmp(line, "vec4i")) { v->type = SAS_INT; v->size = 4; }
            else if (!strcmp(line, "vec2b")) { v->type = SAS_BOOL; v->size = 2; }
            else if (!strcmp(line, "vec3b")) { v->type = SAS_BOOL; v->size = 3; }
            else if (!strcmp(line, "vec4b")) { v->type = SAS_BOOL; v->size = 4; }
            else if (!strcmp(line, "mat2")) { v->type = SAS_FLOAT; v->size = 4; }
            else if (!strcmp(line, "mat3")) { v->type = SAS_FLOAT; v->size = 9; }
            else if (!strcmp(line, "mat4")) { v->type = SAS_FLOAT; v->size = 16; }
            else
            {
                shaders[id]->log = malloc(128);
                sprintf(shaders[id]->log, "Unknown varying type %s.", line);
                shaders[id]->log_len = strlen(shaders[id]->log) + 1;

                fclose(tfp);

                remove("/tmp/.soft-and-slow.varyings");
                remove("/tmp/.soft-and-slow.cpp");
                return;
            }

            if (fgets(line, 256, tfp) == NULL)
            {
                free(v->identifier);
                free(v);
                break;
            }

            if (line[strlen(line) - 1] == '\n')
                line[strlen(line) - 1] = 0;

            v->identifier = strdup(line);


            *lvp = v;
            lvp = &v->next;
        }

        fclose(tfp);
        remove("/tmp/.soft-and-slow.varyings");
    }


    if (system("g++ -Wall -Wextra -std=gnu++0x -O3 -fPIC -c /tmp/.soft-and-slow.cpp -o /tmp/.soft-and-slow.o &> /tmp/.soft-and-slow.complog"))
    {
        tfp = fopen("/tmp/.soft-and-slow.complog", "r");
        if (tfp != NULL)
        {
            fseek(tfp, 0, SEEK_END);
            shaders[id]->log_len = ftell(tfp) + 1;
            rewind(tfp);

            shaders[id]->log = malloc(shaders[id]->log_len);
            fread(shaders[id]->log, 1, shaders[id]->log_len - 1, tfp);

            shaders[id]->log[shaders[id]->log_len - 1] = 0;

            fclose(tfp);

            remove("/tmp/.soft-and-slow.complog");
        }

        remove("/tmp/.soft-and-slow.cpp");
        return;
    }


    // Includes are required by general-shader-common.cpp
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
    {
        dlclose(programs[id]->dl);

        struct program_varyings *v = programs[id]->varyings;
        while (v != NULL)
        {
            struct program_varyings *vn = v->next;
            free(v->saved_values);
            free(v);
            v = vn;
        }
    }


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


    FILE *tfp = fopen("/tmp/.soft-and-slow-common.cpp", "w");
    fwrite(&_binary_general_shader_common_cpp_start, 1, (size_t)&_binary_general_shader_common_cpp_size, tfp);
    fclose(tfp);


    char cmd[192];
    sprintf(cmd, "g++ -Wall -Wextra -std=gnu++0x -O3 -fPIC -shared /tmp/.soft-and-slow-common.cpp /tmp/.soft-and-slow-0x*.o -o %s", soname);

    if (system(cmd))
    {
        fprintf(stderr, "Linking failed, leaving object files in /tmp/.soft-and-slow.0x*.o.\n");
        return;
    }

    system("rm /tmp/.soft-and-slow-0x*.o /tmp/.soft-and-slow-common.cpp");


    programs[id]->dl = dlopen(soname, RTLD_NOW);

    if (programs[id]->dl == NULL)
    {
        fprintf(stderr, "Could not load shared library (%s).\n", dlerror());
        return;
    }


    struct program_varyings **vp = &programs[id]->varyings;
    for (struct sas_shader_list *sl = programs[id]->shaders; sl != NULL; sl = sl->next)
    {
        struct shader_varyings *sv = sl->shader->varyings;

        while (sv != NULL)
        {
            char real_identifier[strlen(sv->identifier) + 13];
            strcpy(real_identifier, "sas_varying_");
            strcat(real_identifier, sv->identifier);

            void *addr = dlsym(programs[id]->dl, real_identifier);
            assert(addr);


            struct program_varyings *pv = calloc(1, sizeof(*pv));
            pv->address = addr;
            pv->type = sv->type;
            pv->size = sv->size;

            *vp = pv;
            vp = &pv->next;


            sv = sv->next;
        }
    }
}

static void sas_delete_shader(struct sas_shader *s)
{
    if (!--s->refcount)
    {
        struct shader_varyings *v = s->varyings;
        while (v != NULL)
        {
            struct shader_varyings *vn = v->next;
            free(v->identifier);
            free(v);
            v = vn;
        }

        free(s->source);
        free(s->compiled);
        free(s->log);
        free(s);
    }
}

void glDeleteShader(GLuint id)
{
    if (shaders[id] == NULL)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }

    sas_delete_shader(shaders[id]);

    shaders[id] = NULL;
}


void glDeleteProgram(GLuint id)
{
    if (programs[id] == NULL)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    struct sas_shader_list *sl = programs[id]->shaders;
    while (sl != NULL)
    {
        sas_delete_shader(sl->shader);

        struct sas_shader_list *n = sl->next;
        free(sl);
        sl = n;
    }


    dlclose(programs[id]->dl);


    struct program_varyings *pv = programs[id]->varyings;
    while (pv != NULL)
    {
        struct program_varyings *pvn = pv->next;
        free(pv->saved_values);
        free(pv);
        pv = pvn;
    }


    free(programs[id]);

    programs[id] = NULL;
}


void glUseProgram(GLuint id)
{
    if (programs[id] == NULL)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    sas_vertex_transformation   = (void (*)(void))(uintptr_t)dlsym(programs[id]->dl, "sas_vertex_transform");
    sas_fragment_transformation = (void (*)(void))(uintptr_t)dlsym(programs[id]->dl, "sas_fragment_transform");

    current_program = programs[id];
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

void glUniform1f(GLint location, GLfloat v0)
{
    GLuint prog = location / SAS_MAX_UNIFORMS;
    location %= SAS_MAX_UNIFORMS;

    if ((programs[prog] == NULL) || (programs[prog]->uniforms[location] == NULL))
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    ((GLfloat *)programs[prog]->uniforms[location])[0] = v0;
}

void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    GLuint prog = location / SAS_MAX_UNIFORMS;
    location %= SAS_MAX_UNIFORMS;

    if ((programs[prog] == NULL) || (programs[prog]->uniforms[location] == NULL))
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    ((GLfloat *)programs[prog]->uniforms[location])[0] = v0;
    ((GLfloat *)programs[prog]->uniforms[location])[1] = v1;
    ((GLfloat *)programs[prog]->uniforms[location])[2] = v2;
}


void glGetShaderiv(GLuint id, GLenum pname, GLint *params)
{
    if (shaders[id] == NULL)
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    switch (pname)
    {
        case GL_SHADER_TYPE:
            *params = shaders[id]->type;
            return;
        case GL_COMPILE_STATUS:
            *params = shaders[id]->compiled_len ? GL_TRUE : GL_FALSE;
            return;
        case GL_INFO_LOG_LENGTH:
            *params = shaders[id]->log_len;
            return;
    }

    sas_error = GL_INVALID_ENUM;
}

void glGetShaderInfoLog(GLuint id, GLsizei max_length, GLsizei *length, GLchar *log)
{
    if ((shaders[id] == NULL) || (max_length <= 0))
    {
        sas_error = GL_INVALID_VALUE;
        return;
    }


    if ((unsigned)max_length >= shaders[id]->log_len)
    {
        memcpy(log, shaders[id]->log, shaders[id]->log_len);

        if (length != NULL)
            *length = shaders[id]->log_len - 1;
    }
    else
    {
        memcpy(log, shaders[id]->log, --max_length);
        log[max_length] = 0;

        if (length != NULL)
            *length = max_length;
    }
}


void sas_varyings_alloc(size_t sz)
{
    for (struct program_varyings *pv = current_program->varyings; pv != NULL; pv = pv->next)
    {
        free(pv->saved_values);

        pv->saved_values = malloc(sz * varying_type_sizes[pv->type] * pv->size);
    }

    varying_index = 0;
}

void sas_flush_varyings(void)
{
    varying_index = 0;
}

void sas_flush_varyings_partially(size_t sz)
{
    varying_index -= sz;

    for (struct program_varyings *pv = current_program->varyings; pv != NULL; pv = pv->next)
        memmove(pv->saved_values, (void *)((uintptr_t)pv->saved_values + varying_index * varying_type_sizes[pv->type] * pv->size), sz * varying_type_sizes[pv->type] * pv->size);
}

void sas_push_varyings(void)
{
    for (struct program_varyings *pv = current_program->varyings; pv != NULL; pv = pv->next)
        memcpy((void *)((uintptr_t)pv->saved_values + varying_index * varying_type_sizes[pv->type] * pv->size), pv->address, varying_type_sizes[pv->type] * pv->size);

    varying_index++;
}

// TODO: Optimize; this one is called really often (when using other varyings
// than the built-in ones).
void sas_calc_varyings(int i1, int i2, int i3, float w1, float w2, float w3, float dd)
{
    for (struct program_varyings *pv = current_program->varyings; pv != NULL; pv = pv->next)
    {
        int sz = pv->size;

#ifdef USE_ASSEMBLY
        if ((pv->size == 1) && (pv->type == SAS_XMM))
        {
            // Treat that case as a special one. (Hint: Could we save the
            // pshufd'd wX/dd in some XMM register? Would that be of any good?)
            // (FIXME btw: Add another 4-float type instead of that sas_color_t
            // stuff)
            __asm__ __volatile__ ("pshufd xmm0,%4,0x00;"
                                  "mulps  xmm0,%1;"
                                  "pshufd xmm1,%5,0x00;"
                                  "mulps  xmm1,%2;"
                                  "addps  xmm0,xmm1;"
                                  "pshufd xmm1,%6,0x00;"
                                  "mulps  xmm1,%3;"
                                  "addps  xmm0,xmm1;"
                                  "pshufd xmm1,%7,0x00;"
                                  "mulps  xmm0,xmm1;"
                                  "movaps %0,xmm0"
                                  : "=m"(*pv->val_xmm)
                                  : "m"(pv->saved_xmm[i1]), "m"(pv->saved_xmm[i2]), "m"(pv->saved_xmm[i3]), "x"(w1), "x"(w2), "x"(w3), "x"(dd)
                                  : "xmm0", "xmm1");
        }
        else
        {
#endif
            switch (pv->type)
            {
                case SAS_XMM:
                    sz *= 4;
                    for (int i = 0; i < sz; i++)
                        pv->val_float[i] = (pv->saved_float[i1 * sz + i] * w1 + pv->saved_float[i2 * sz + i] * w2 + pv->saved_float[i3 * sz + i] * w3) * dd;
                    break;
                case SAS_FLOAT:
                    for (int i = 0; i < sz; i++)
                        pv->val_float[i] = (pv->saved_float[i1 * sz + i] * w1 + pv->saved_float[i2 * sz + i] * w2 + pv->saved_float[i3 * sz + i] * w3) * dd;
                    break;
                case SAS_INT:
                    for (int i = 0; i < sz; i++)
                    pv->val_int[i] = (int)((pv->saved_int[i1 * sz + i] * w1 + pv->saved_int[i2 * sz + i] * w2 + pv->saved_int[i3 * sz + i] * w3) * dd);
                    break;
                case SAS_BOOL:
                    // TODO: How the Sam Hill are we supposed to work here? Is this
                    // even legal?
                    for (int i = 0; i < sz; i++)
                        pv->val_bool[i] = !!((pv->saved_bool[i1 * sz + i] * w1 + pv->saved_bool[i2 * sz + i] * w2 + pv->saved_bool[i3 * sz + i] * w3) * dd);
                break;
            }
#ifdef USE_ASSEMBLY
        }
#endif
    }
}
