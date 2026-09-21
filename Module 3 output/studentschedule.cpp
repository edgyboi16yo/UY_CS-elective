#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include <string>
using namespace std;

struct Course {
    string code, title, section, units, days, time, room;
};

Course subjects[] = {
    {"CS0011", "MOBILE PROGRAMMING", "TN36", "3", "S / W", "17:00 - 18:50", "F608 / F1209"},
    {"CS0019", "MODELING AND SIMULATION", "TN35", "3", "F / T", "11:00 - 12:50", "ONLINE"},
    {"CS0025", "SOFTWARE ENGINEERING 1", "TN35", "3", "F / TH", "15:00 - 16:50", "ONLINE / E609"},
    {"CS0045", "CS ELECTIVE - COMPUTER GRAPHICS AND VISUAL COMPUTING", "TN35", "3", "M / TH", "13:00 - 14:50", "E601 / E610"},
    {"CS0053", "CS SPECIALIZATION 2 - PROGRAMMING TOOLS AND TECHNIQUES", "TN35", "3", "M / TH", "07:00 - 08:50", "F702 / F609"}
};

void color(float r, float g, float b) { glColor3f(r, g, b); }

void box(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1,y1); glVertex2f(x2,y1);
    glVertex2f(x2,y2); glVertex2f(x1,y2);
    glEnd();
}

void border(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1,y1); glVertex2f(x2,y1);
    glVertex2f(x2,y2); glVertex2f(x1,y2);
    glEnd();
}

void text(float x, float y, string value, void* font = GLUT_BITMAP_HELVETICA_12) {
    glRasterPos2f(x, y);
    for (char letter : value) glutBitmapCharacter(font, letter);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    color(0.02f, 0.27f, 0.18f);
    box(0, 0, 1200, 90);
    color(0.95f, 0.73f, 0.10f);
    box(0, 86, 1200, 90);
    color(1, 1, 1);
    text(40, 42, "STUDENT SUBJECT SCHEDULE", GLUT_BITMAP_HELVETICA_18);
    color(0.75f, 0.90f, 0.82f);
    text(40, 67, "1st Term | School Year 2026-2027");
    text(985, 54, "FEU TECH", GLUT_BITMAP_HELVETICA_18);

    color(1, 1, 1);
    box(40, 115, 1160, 250);
    color(0.02f, 0.40f, 0.24f);
    box(40, 115, 50, 250);
    color(0.75f, 0.80f, 0.77f);
    border(40, 115, 1160, 250);

    color(0.04f, 0.30f, 0.19f);
    text(70, 145, "STUDENT INFORMATION", GLUT_BITMAP_HELVETICA_18);
    color(0.12f, 0.15f, 0.14f);
    text(70, 178, "Student Number: 202411597");
    text(70, 203, "Name: UY, TIMOTHY MICHAEL DELA CRUZ");
    text(70, 228, "Phone: +639451004863");

    text(600, 178, "College: COMPUTER STUDIES");
    text(600, 203, "Program: BSCSSE");
    text(600, 228, "Year Level: 2");

    color(0.10f, 0.55f, 0.29f);
    box(970, 145, 1125, 205);
    color(1, 1, 1);
    text(993, 169, "ENROLLMENT");
    text(1008, 193, "ENROLLED", GLUT_BITMAP_HELVETICA_18);

    color(0.04f, 0.30f, 0.19f);
    text(40, 283, "ENROLLED COURSES", GLUT_BITMAP_HELVETICA_18);
    box(40, 300, 1160, 340);
    color(1, 1, 1);
    text(55, 325, "CODE"); text(145, 325, "COURSE TITLE");
    text(675, 325, "SECTION"); text(755, 325, "UNITS");
    text(815, 325, "DAYS"); text(885, 325, "TIME");
    text(1055, 325, "ROOM");

    for (int i = 0; i < 5; i++) {
        float top = 340 + i * 43;
        if (i % 2 == 0) color(1, 1, 1);
        else color(0.92f, 0.96f, 0.93f);
        box(40, top, 1160, top + 43);
        color(0.72f, 0.78f, 0.74f);
        border(40, top, 1160, top + 43);
        color(0.10f, 0.13f, 0.12f);
        text(55, top+26, subjects[i].code, GLUT_BITMAP_HELVETICA_10);
        text(145, top+26, subjects[i].title, GLUT_BITMAP_HELVETICA_10);
        text(687, top+26, subjects[i].section, GLUT_BITMAP_HELVETICA_10);
        text(770, top+26, subjects[i].units, GLUT_BITMAP_HELVETICA_10);
        text(815, top+26, subjects[i].days, GLUT_BITMAP_HELVETICA_10);
        text(885, top+26, subjects[i].time, GLUT_BITMAP_HELVETICA_10);
        text(1055, top+26, subjects[i].room, GLUT_BITMAP_HELVETICA_10);
    }

    color(0.95f, 0.73f, 0.10f);
    box(40, 555, 1160, 605);
    color(0.15f, 0.13f, 0.05f);
    text(55, 586, "TOTAL UNITS: 15", GLUT_BITMAP_HELVETICA_18);
    text(950, 586, "STATUS: ENROLLED");
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1200, 650, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1200, 650);
    glutCreateWindow("Student Subject Schedule");
    glClearColor(0.94f, 0.96f, 0.95f, 1);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}