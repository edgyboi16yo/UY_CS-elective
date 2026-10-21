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
   * Example 16 - Continuous VBO Animation via glMapBuffer + Timer
   * ------------------------------------------------------------
   * Concept: Combines glutTimerFunc (Module 3) with glMapBuffer to
   * animate a shape that lives ENTIRELY in GPU memory. Every second, the
   * timer callback maps the buffer, nudges the triangle sideways, and
   * unmaps it - the same pattern as the lecture's modifyTriangle(),
   * running continuously via a self-rescheduling timer.
   */

  GLuint VBOid;
  bool triangleReady = false;





void triangle() {
    if (!triangleReady) {
        GLfloat trianglevertices[] = {
            0.0f, .75f, 0.0f,
            -0.75f, 0.0f, 0.0f,
            0.75f, 0.0f, 0.0f
        };
        glGenBuffers(1, &VBOid);
        glBindBuffer(GL_ARRAY_BUFFER, VBOid);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, trianglevertices,
GL_DYNAMIC_DRAW);
        triangleReady = true;
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor4f(.16f, .72f, .08f, 1.0f);
    triangle();
    glFlush();
}

void animateStep(int value) {
    static float dx = 0.02f;

    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    GLfloat* mapped = (GLfloat*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    if (!mapped) { cerr << "Cannot map animation buffer." << endl; exit(1); }
    if (mapped[0] > 0.3f || mapped[0] < -0.3f) dx = -dx;
    mapped[0] += dx;
    mapped[3] += dx;
    mapped[6] += dx;
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glutPostRedisplay();
    glutTimerFunc(200, animateStep, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Ex16 - Continuous Animation via glutTimerFunc + glMapBuffer");

    glutDisplayFunc(display);




       glutTimerFunc(200, animateStep, 0);

  #ifndef __APPLE__
      GLenum err = glewInit();
      if (err != GLEW_OK) {
        cerr << "GLEW initialization failed." << endl;
        return 1;
    }
    if (err == GLEW_OK) {
        triangle(); // Create buffers before any timer can use them.
          glutMainLoop();
      }
  #else
      glutMainLoop();
  #endif
      return 0;
  }
