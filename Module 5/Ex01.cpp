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
   * Example 01 - Immediate Mode Baseline (Before Conversion)
   * ------------------------------------------------------------
   * Concept: A plain glBegin()/glEnd() triangle. This is the starting
   * point Example 02 converts into a Vertex Buffer Object (VBO), so you
   * can compare both approaches side by side.
   */

  void triangle() {
      glBegin(GL_TRIANGLES);
          glVertex2f(0.0f, 0.75f);
          glVertex2f(-0.75f, 0.0f);
          glVertex2f(0.75f, 0.0f);
      glEnd();
  }

  void display() {
      glClear(GL_COLOR_BUFFER_BIT);
      glColor4f(.16f, .72f, .08f, 1.0f);
      triangle();
      glFlush();
  }

  int main(int argc, char** argv) {
      glutInit(&argc, argv);
      glutInitWindowSize(800, 600);
      glutInitWindowPosition(100, 100);
      glutCreateWindow("Ex01 - Immediate Mode Baseline (Before Conversion)");





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
