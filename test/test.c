#include <math.h>
#include <stdio.h>

#include <soft-and-slow/context.h>
#include <soft-and-slow/gl.h>
#include <soft-and-slow/glu.h>


void write_color_buffer(const char *file, sas_context_t c)
{
    FILE *fp = fopen(file, "w");

    fprintf(fp, "P6\n%u %u\n255\n", c->width, c->height);

    for (unsigned y = 0; y < c->height; y++)
    {
        for (unsigned x = 0; x < c->width; x++)
        {
            fputc(c->colorbuffer[y * c->width + x].r, fp);
            fputc(c->colorbuffer[y * c->width + x].g, fp);
            fputc(c->colorbuffer[y * c->width + x].b, fp);
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

int main(void)
{
    sas_context_t ctx = create_sas_context(640, 480);
    set_current_sas_context(ctx);


    glMatrixMode(GL_PROJECTION);
    gluPerspective(45., 640. / 480., 1., 100.);

    glMatrixMode(GL_MODELVIEW);


    glEnable(GL_DEPTH_TEST);

    glClearColor(0.f, 0.f, 0.f, 1.f);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glTranslatef(0.f, 0.f, -10.f);
    glPushMatrix();

    glTranslatef(-2.f, 0.f, 0.f);

    glBegin(GL_POINTS);
    glTexCoord2f(1.f, 1.f);
    glColor3f(1.f, 0.f, 0.f); glVertex3f(-1.f, -1.f, 0.f);
    glColor3f(0.f, 0.f, 1.f); glVertex3f( 1.f, -1.f, 0.f);
    glColor3f(0.f, 1.f, 0.f); glVertex3f( 0.f,  1.f, 0.f);
    glEnd();

    glPopMatrix();
    glTranslatef(2.f, 0.f, 0.f);

    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.f, 0.f); glColor3f(1.f, 0.f, 0.f); glVertex3f(-1.f, -1.f, 0.f);
    glTexCoord2f(1.f, 0.f); glColor3f(0.f, 0.f, 1.f); glVertex3f( 1.f, -1.f, 0.f);
    glTexCoord2f(.5f, 1.f); glColor3f(0.f, 1.f, 0.f); glVertex3f( 0.f,  1.f, 0.f);
    glEnd();


    write_color_buffer("color.ppm", ctx);
    write_depth_buffer("depth.pgm", ctx);


    return 0;
}
