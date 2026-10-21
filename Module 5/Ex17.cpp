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
   * Example 17 - Indexed Drawing With BOTH a Color VBO and an Element VBO
   * ------------------------------------------------------------
   * Concept: Combines Example 10's shared-vertex index buffer with
   * Example 11's separate color buffer. A quad is described by 4 unique
   * positions, 4 unique colors, and one 6-entry index buffer -
   * everything GPU-resident, everything drawn with ONE glDrawElements call.
   */

  GLuint posVBO, colorVBO, indexVBO;

  void coloredIndexedQuad() {
      GLfloat positions[] = {
          -0.5f, -0.5f, 0.0f,
           0.5f, -0.5f, 0.0f,
           0.5f, 0.5f, 0.0f,




        -0.5f, 0.5f, 0.0f
    };
    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f
    };
    GLubyte indices[] = { 0, 1, 2, 0, 2, 3 };

    glGenBuffers(1, &posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glGenBuffers(1, &indexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glColorPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &posVBO);
    glDeleteBuffers(1, &colorVBO);
    glDeleteBuffers(1, &indexVBO);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    coloredIndexedQuad();
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);





        glutInitWindowPosition(100, 100);
        glutCreateWindow("Ex17 - Indexed Drawing With BOTH a Color VBO and an Element VBO");

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
