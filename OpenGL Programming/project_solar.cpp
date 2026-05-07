#include <GL/glut.h>
#include <math.h>

float earthRev = 0, earthRot = 0;
float moonRev = 0;
float marsRev = 0, marsRot = 0;

void init() {
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
}

void drawSphere(float r, float R, float G, float B) {
    glColor3f(R, G, B);
    glutSolidSphere(r, 50, 50);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0, 8, 20, 0, 0, 0, 0, 1, 0);

    // 🌞 SUN
    glPushMatrix();
    drawSphere(1.5, 1.0, 0.6, 0.0);
    glPopMatrix();

    // 🌍 EARTH REVOLUTION
    glPushMatrix();
    glRotatef(earthRev, 0, 1, 0);
    glTranslatef(6, 0, 0);

    // Earth self-rotation
    glRotatef(earthRot, 0, 1, 0);
    drawSphere(0.5, 0.2, 0.5, 1.0);

        // 🌙 MOON
        glPushMatrix();
        glRotatef(moonRev, 0, 1, 0);
        glTranslatef(1.2, 0, 0);
        drawSphere(0.2, 0.8, 0.8, 0.8);
        glPopMatrix();

    glPopMatrix();

    // 🔴 MARS
    glPushMatrix();
    glRotatef(marsRev, 0, 1, 0);
    glTranslatef(-10, 0, 0);

    glRotatef(marsRot, 0, 1, 0);
    drawSphere(0.4, 1.0, 0.3, 0.2);

    glPopMatrix();

    glutSwapBuffers();
}

void timer(int value) {
    earthRev += 0.2;
    earthRot += 2.0;
    moonRev += 1.0;

    marsRev += 0.1;
    marsRot += 1.5;

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)w/h, 1, 100);

    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("Solar System Simulation - OpenGL");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
