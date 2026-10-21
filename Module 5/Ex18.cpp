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
   * Example 18 - A Composite Scene, Every Shape a Separate VBO
   * ------------------------------------------------------------
   * Concept: A small scene combining three independently managed VBOs -
   * a triangle "roof", a quad "wall", and a GL_POLYGON "sun" - each
   * generated, bound, drawn, and released in its own turn.
   */

  GLuint roofVBO, wallVBO, sunVBO;

  void roof() {




    GLfloat v[] = { 0.0f, 0.6f, 0.0f, -0.5f, 0.1f, 0.0f, 0.5f, 0.1f, 0.0f };
    glGenBuffers(1, &roofVBO);
    glBindBuffer(GL_ARRAY_BUFFER, roofVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glColor3f(0.7f, 0.2f, 0.2f);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void wall() {
    GLfloat v[] = { -0.5f, 0.1f, 0.0f, 0.5f, 0.1f, 0.0f, 0.5f, -0.5f, 0.0f,    -0.5f, -
0.5f, 0.0f };
    glGenBuffers(1, &wallVBO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glColor3f(0.85f, 0.7f, 0.5f);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void sun() {
    GLfloat v[] = {
        0.75f, 0.7f, 0.0f, 0.65f, 0.6f, 0.0f, 0.75f, 0.55f, 0.0f,
        0.85f, 0.6f, 0.0f, 0.8f, 0.72f, 0.0f
    };
    glGenBuffers(1, &sunVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glColor3f(1.0f, 0.85f, 0.1f);
    glDrawArrays(GL_POLYGON, 0, 5);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    wall();
    roof();
    sun();
    glDeleteBuffers(1, &roofVBO);
    glDeleteBuffers(1, &wallVBO);
    glDeleteBuffers(1, &sunVBO);
    glFlush();
}





  int main(int argc, char** argv) {
      glutInit(&argc, argv);
      glutInitWindowSize(800, 600);
      glutInitWindowPosition(100, 100);
      glutCreateWindow("Ex18 - Multi-Shape Scene From Three Independent VBOs");

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
