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
   * Example 11 - A Second VBO for Per-Vertex Color
   * ------------------------------------------------------------
   * Concept: Exactly as position data can live in a GPU-resident VBO,
   * so can color data. TWO separate buffers are generated, bound, and
   * filled in turn; both GL_VERTEX_ARRAY and GL_COLOR_ARRAY client
   * states are enabled together for the draw call.
   */

  GLuint posVBO, colorVBO;

  void coloredTriangle() {
      GLfloat positions[] = {
          0.0f, 0.75f, 0.0f,
          -0.75f, 0.0f, 0.0f,
          0.75f, 0.0f, 0.0f
      };
      GLfloat colors[] = {




         1.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 1.0f
    };

    glGenBuffers(1, &posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, positions, GL_STATIC_DRAW);

    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, colors, GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glColorPointer(3, GL_FLOAT, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &posVBO);
    glDeleteBuffers(1, &colorVBO);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    coloredTriangle();
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Ex11 - Position VBO + Color VBO Together");

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
