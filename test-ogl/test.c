#include <stddef.h>

#include <SFML/Graphics.h>
#include <SFML/OpenGL.h>
#include <SFML/System.h>
#include <SFML/Window.h>


int main(void)
{
    sfWindow *wnd = sfWindow_Create((sfVideoMode){ 640, 480, 32 }, "test", sfTitlebar | sfClose, NULL);


    glMatrixMode(GL_PROJECTION);
    gluPerspective(45., 640. / 480., 1., 100.);

    glMatrixMode(GL_MODELVIEW);


    glEnable(GL_DEPTH_TEST);

    glClearColor(0.f, 0.f, 0.f, 1.f);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glTranslatef(0.f, 0.f, -6.f);
    glPushMatrix();

    glTranslatef(-1.5f, 0.f, 0.f);

    glBegin(GL_TRIANGLES);
    glColor3f(1.f, 0.f, 0.f); glVertex3f(-1.f, -1.f, 0.f);
    glColor3f(0.f, 0.f, 1.f); glVertex3f( 1.f, -1.f, 0.f);
    glColor3f(0.f, 1.f, 0.f); glVertex3f( 0.f,  1.f, 0.f);
    glEnd();

    glPopMatrix();

    glEnable(GL_TEXTURE_2D);

    sfTexture *t = sfTexture_CreateFromFile("checkerboard.png", NULL);
    sfTexture_SetSmooth(t, 0);
    sfTexture_Bind(t);

    glTranslatef(1.5f, 0.f, 0.f);

    glRotatef(45.f, -1.f, 0.f, 0.f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.f, 0.f); glColor3f(1.f, 0.f, 0.f); glVertex3f(-1.f, -1.f, 0.f);
    glTexCoord2f(0.f, 1.f); glColor3f(0.f, 0.f, 1.f); glVertex3f(-1.f,  1.f, 0.f);
    glTexCoord2f(1.f, 1.f); glColor3f(0.f, 1.f, 0.f); glVertex3f( 1.f,  1.f, 0.f);
    glTexCoord2f(1.f, 0.f); glColor3f(1.f, 1.f, 0.f); glVertex3f( 1.f, -1.f, 0.f);
    glEnd();


    for (;;)
        sfWindow_Display(wnd);


    return 0;
}
