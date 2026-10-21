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
 * Example 13 - The Colored Pinwheel, Rendered From VBOs
 * ------------------------------------------------------------
 * Concept: Module 4's four-triangle pinwheel scene, rebuilt so BOTH
 * the position data and the color data live in GPU-resident VBOs
 * instead of plain client-side arrays.
 */

GLuint posVBO, colorVBO;

void pinwheel() {
    GLfloat positions[] = {
        0.0f, 0.0f, 0.0f,   -0.5f, 0.10f, 0.0f,  -0.5f, -0.10f, 0.0f,
        0.0f, 0.0f, 0.0f,    0.5f, 0.10f, 0.0f,   0.5f, -0.10f, 0.0f,
        0.0f, 0.0f, 0.0f,   -0.10f, 0.50f, 0.0f,  0.10f, 0.50f, 0.0f,
        0.0f, 0.0f, 0.0f,   -0.10f, -0.50f, 0.0f, 0.10f, -0.50f, 0.0f
    };
    GLfloat colors[] = {
        1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f,
        1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f,
        1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f,
        1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f
    };

   glGenBuffers(1, &posVBO);
   glBindBuffer(GL_ARRAY_BUFFER, posVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

   glGenBuffers(1, &colorVBO);
   glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);

   glBindBuffer(GL_ARRAY_BUFFER, posVBO);
   glVertexPointer(3, GL_FLOAT, 0, 0);
   glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
   glColorPointer(3, GL_FLOAT, 0, 0);





      glDrawArrays(GL_TRIANGLES, 0, 12);

      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glDeleteBuffers(1, &posVBO);
      glDeleteBuffers(1, &colorVBO);
  }

  void display() {
      glClear(GL_COLOR_BUFFER_BIT);
      pinwheel();
      glFlush();
  }

  int main(int argc, char** argv) {
      glutInit(&argc, argv);
      glutInitWindowSize(800, 600);
      glutInitWindowPosition(100, 100);
      glutCreateWindow("Ex13 - VBO Pinwheel");

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
