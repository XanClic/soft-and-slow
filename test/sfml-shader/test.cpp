#include <cstddef>
#include <cstdio>

#include <sys/time.h>

#ifdef SAS
#include <soft-and-slow/context.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/glext.h>
#include <soft-and-slow/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#endif

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#ifdef SAS
#include <SDL/SDL.h>
#endif


#ifdef SAS
void write_color_buffer(const char *file, sas_context_t c)
{
    FILE *fp = fopen(file, "w");

    fprintf(fp, "P6\n%u %u\n255\n", c->width, c->height);

    for (unsigned y = 0; y < c->height; y++)
    {
        for (unsigned x = 0; x < c->width; x++)
        {
            fputc(c->colorbuffer[y * c->width + x].channels.r, fp);
            fputc(c->colorbuffer[y * c->width + x].channels.g, fp);
            fputc(c->colorbuffer[y * c->width + x].channels.b, fp);
        }
    }

    fclose(fp);
}

void write_depth_buffer(const char *file, sas_context_t c)
{
    FILE *fp = fopen(file, "w");

    fprintf(fp, "P5\n%u %u\n255\n", c->width, c->height);

    for (unsigned y = 0; y < c->height; y++)
        for (unsigned x = 0; x < c->width; x++)
            fputc(lrintf(c->depthbuffer[y * c->width + x] * 255.f), fp);

    fclose(fp);
}
#endif


static struct timeval tv1, tv2;

static void start_time(void)
{
    gettimeofday(&tv1, NULL);
}

static void stop_time(void)
{
    gettimeofday(&tv2, NULL);

    int secs = tv2.tv_sec - tv1.tv_sec, usecs = tv2.tv_usec - tv1.tv_usec;
    if (usecs < 0)
    {
        secs--;
        usecs += 1000000;
    }
    printf("Time difference: %i.%06i s\n", secs, usecs);
}

static float frame_time(void)
{
    gettimeofday(&tv2, NULL);

    float secs = tv2.tv_sec - tv1.tv_sec + (tv2.tv_usec - tv1.tv_usec) / 1e6;

    tv1 = tv2;

    return secs;
}


int main(void)
{
#ifdef SAS
    sas_context_t ctx = create_sas_context(640, 480);
    set_current_sas_context(ctx);
#endif

#ifdef SAS
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Surface *sfc = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    ctx->colorbuffer = (SAS_COLOR_TYPE *)sfc->pixels;

    SDL_WM_SetCaption("S&S GL", "S&S GL");
#else
    sf::Window wnd(sf::VideoMode(640, 480), "S&S GL -- OpenGL compare");
#endif


    start_time();

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45., 640. / 480., 1., 100.);

    glMatrixMode(GL_MODELVIEW);


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    glClearColor(0.f, 0.f, 0.f, 1.f);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glTranslatef(0.f, 0.f, -3.f);

    stop_time();


    sf::Texture blend_text, text1, text2, text3;
    blend_text.LoadFromFile("blend.jpg");
    text1.LoadFromFile("text1.png");
    text2.LoadFromFile("text2.png");
    text3.LoadFromFile("text3.png");

    glActiveTexture(GL_TEXTURE0);
    blend_text.Bind();

    glActiveTexture(GL_TEXTURE1);
    text1.Bind();

    glActiveTexture(GL_TEXTURE2);
    text2.Bind();

    glActiveTexture(GL_TEXTURE3);
    text3.Bind();



    GLuint prog = glCreateProgram();


    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);

    FILE *fp = fopen("vertex.glsl", "r");
    fseek(fp, 0, SEEK_END);
    int vs_len = ftell(fp);
    rewind(fp);

    char *vs_src = (char *)malloc(vs_len);
    fread(vs_src, vs_len, 1, fp);
    fclose(fp);

    glShaderSource(vert_shader, 1, (const GLchar **)&vs_src, &vs_len);
    free(vs_src);

    glCompileShader(vert_shader);
    glAttachShader(prog, vert_shader);
    glDeleteShader(vert_shader);


    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    fp = fopen("fragment.glsl", "r");
    fseek(fp, 0, SEEK_END);
    int fs_len = ftell(fp);
    rewind(fp);

    char *fs_src = (char *)malloc(fs_len);
    fread(fs_src, fs_len, 1, fp);
    fclose(fp);

    glShaderSource(frag_shader, 1, (const GLchar **)&fs_src, &fs_len);
    free(fs_src);

    glCompileShader(frag_shader);
    glAttachShader(prog, frag_shader);
    glDeleteShader(frag_shader);


    glLinkProgram(prog);
    glUseProgram(prog);


    glUniform1i(glGetUniformLocation(prog, "blend_text"), 0);
    glUniform1i(glGetUniformLocation(prog, "text1"),      1);
    glUniform1i(glGetUniformLocation(prog, "text2"),      2);
    glUniform1i(glGetUniformLocation(prog, "text3"),      3);


    frame_time();

    bool quit = false;

    float ft_accum = 0.f;
    int ft_accum_i = 0;

    while (!quit)
    {
        float ft = frame_time();


        glRotatef(ft * 50.f, -1.f, .34f, .6f);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBegin(GL_QUADS);
        glColor3f(1.f, 1.f, 1.f);

        glTexCoord2f(0.f, 1.f); glVertex3f(-.5f, -.5f,  .5f);
        glTexCoord2f(0.f, 0.f); glVertex3f(-.5f,  .5f,  .5f);
        glTexCoord2f(1.f, 0.f); glVertex3f( .5f,  .5f,  .5f);
        glTexCoord2f(1.f, 1.f); glVertex3f( .5f, -.5f,  .5f);

        glTexCoord2f(0.f, 1.f); glVertex3f( .5f, -.5f, -.5f);
        glTexCoord2f(0.f, 0.f); glVertex3f( .5f,  .5f, -.5f);
        glTexCoord2f(1.f, 0.f); glVertex3f(-.5f,  .5f, -.5f);
        glTexCoord2f(1.f, 1.f); glVertex3f(-.5f, -.5f, -.5f);

        glTexCoord2f(1.f, 0.f); glVertex3f( .5f,  .5f, -.5f);
        glTexCoord2f(1.f, 1.f); glVertex3f( .5f,  .5f,  .5f);
        glTexCoord2f(0.f, 1.f); glVertex3f(-.5f,  .5f,  .5f);
        glTexCoord2f(0.f, 0.f); glVertex3f(-.5f,  .5f, -.5f);

        glTexCoord2f(0.f, 1.f); glVertex3f( .5f, -.5f,  .5f);
        glTexCoord2f(0.f, 0.f); glVertex3f( .5f, -.5f, -.5f);
        glTexCoord2f(1.f, 0.f); glVertex3f(-.5f, -.5f, -.5f);
        glTexCoord2f(1.f, 1.f); glVertex3f(-.5f, -.5f,  .5f);

        glTexCoord2f(0.f, 1.f); glVertex3f(-.5f, -.5f, -.5f);
        glTexCoord2f(0.f, 0.f); glVertex3f(-.5f,  .5f, -.5f);
        glTexCoord2f(1.f, 0.f); glVertex3f(-.5f,  .5f,  .5f);
        glTexCoord2f(1.f, 1.f); glVertex3f(-.5f, -.5f,  .5f);

        glTexCoord2f(0.f, 1.f); glVertex3f( .5f, -.5f,  .5f);
        glTexCoord2f(0.f, 0.f); glVertex3f( .5f,  .5f,  .5f);
        glTexCoord2f(1.f, 0.f); glVertex3f( .5f,  .5f, -.5f);
        glTexCoord2f(1.f, 1.f); glVertex3f( .5f, -.5f, -.5f);
        glEnd();


#ifdef SAS
        SDL_UpdateRect(sfc, 0, 0, 0, 0);
#else
        wnd.Display();
#endif


#ifdef SAS
        SDL_Event evt;

        while (SDL_PollEvent(&evt))
            if (evt.type == SDL_QUIT)
                quit = true;
#else
        sf::Event evt;

        while (wnd.PollEvent(evt))
            if (evt.Type == sf::Event::Closed)
                quit = true;
#endif


        ft_accum += ft;
        if (++ft_accum_i == 10)
        {
            printf("%10g FPS\r", 10.f / ft_accum);
            fflush(stdout);

            ft_accum_i = 0;
            ft_accum = 0.f;
        }
    }


#ifdef SAS
    write_color_buffer("color.ppm", ctx);
    write_depth_buffer("depth.pgm", ctx);
#endif


    return 0;
}
