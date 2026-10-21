#define GL_SILENCE_DEPRECATION
  #ifdef __APPLE__
  #include <GLUT/glut.h>
  #else
  #include <GL/glew.h>
  #include <GL/glut.h>
  #endif
  #include <iostream>
  #include <cmath>
  #include <cstdlib>
  using namespace std;

  /*
   * Example 19 - Batching Many Triangles Into ONE VBO
   * ------------------------------------------------------------
   * Concept: The real-world payoff of VBOs shows up at scale. Here, 50
   * small triangles are generated procedurally into ONE large array,
   * uploaded to the GPU in ONE glBufferData call, and rendered with ONE
   * glDrawArrays call - instead of 50 separate immediate-mode





* glBegin/glEnd blocks or 50 separate small VBOs.
*/

const int TRIANGLE_COUNT = 50;
GLuint VBOid;

void batch() {
    GLfloat verts[TRIANGLE_COUNT * 9];
    for (int i = 0; i < TRIANGLE_COUNT; i++) {
        float t = (float)i / TRIANGLE_COUNT;
        float cx = -0.9f + t * 1.8f;
        float cy = 0.3f * sinf(t * 6.28f * 2.0f);
        int b = i * 9;
        verts[b + 0] = cx;        verts[b + 1] = cy + 0.03f; verts[b + 2] = 0.0f;
        verts[b + 3] = cx - 0.02f; verts[b + 4] = cy - 0.02f; verts[b + 5] = 0.0f;
        verts[b + 6] = cx + 0.02f; verts[b + 7] = cy - 0.02f; verts[b + 8] = 0.0f;
    }

    glGenBuffers(1, &VBOid);
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glColor3f(0.2f, 0.8f, 0.9f);
    glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_COUNT * 3);
    glDisableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBOid);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    batch();
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Ex19 - Batching Many Shapes Into One VBO");

    glutDisplayFunc(display);



#ifndef __APPLE__
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cerr << "GLEW initialization failed." << endl;
        return 1;
    }
    if (err == GLEW_OK) {
        glutMainLoop();




      }
  #else
      glutMainLoop();
  #endif
      return 0;
  }
