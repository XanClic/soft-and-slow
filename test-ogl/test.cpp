#include <cstddef>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>


int main(void)
{
    sf::Window *wnd = new sf::Window(sf::VideoMode(640, 480), "test");


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

    sf::Texture t;
    t.LoadFromFile("checkerboard.png");
    t.SetSmooth(0);
    t.Bind();

    glTranslatef(1.5f, 0.f, 0.f);

    glRotatef(45.f, -1.f, 0.f, 0.f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.f, 0.f); glColor3f(1.f, 0.f, 0.f); glVertex3f(-1.f, -1.f, 0.f);
    glTexCoord2f(0.f, 1.f); glColor3f(0.f, 0.f, 1.f); glVertex3f(-1.f,  1.f, 0.f);
    glTexCoord2f(1.f, 1.f); glColor3f(0.f, 1.f, 0.f); glVertex3f( 1.f,  1.f, 0.f);
    glTexCoord2f(1.f, 0.f); glColor3f(1.f, 1.f, 0.f); glVertex3f( 1.f, -1.f, 0.f);
    glEnd();


    for (;;)
        wnd->Display();


    return 0;
}
