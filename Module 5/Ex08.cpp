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
   * Example 08 - Two Separate VBOs, Two Separate Shapes
   * ------------------------------------------------------------
   * Concept: An application is not limited to one VBO. Each shape can
   * have its own buffer ID, generated and bound independently, and
   * drawn in its own turn.
   */

  GLuint leftVBO, rightVBO;

  void triangleLeft() {
      GLfloat verts[] = { -0.7f, 0.4f, 0.0f, -0.9f, -0.2f, 0.0f, -0.5f, -0.2f, 0.0f };
      glGenBuffers(1, &leftVBO);
      glBindBuffer(GL_ARRAY_BUFFER, leftVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, verts, GL_STATIC_DRAW);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, 0);
      glColor3f(1.0f, 0.4f, 0.1f);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glDisableClientState(GL_VERTEX_ARRAY);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
  }




  void triangleRight() {
      GLfloat verts[] = { 0.7f, 0.4f, 0.0f, 0.5f, -0.2f, 0.0f, 0.9f, -0.2f, 0.0f };
      glGenBuffers(1, &rightVBO);
      glBindBuffer(GL_ARRAY_BUFFER, rightVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, verts, GL_STATIC_DRAW);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, 0);
      glColor3f(0.1f, 0.4f, 1.0f);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glDisableClientState(GL_VERTEX_ARRAY);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void display() {
      glClear(GL_COLOR_BUFFER_BIT);
      triangleLeft();
      triangleRight();
      glDeleteBuffers(1, &leftVBO);
      glDeleteBuffers(1, &rightVBO);
      glFlush();
  }

  int main(int argc, char** argv) {
      glutInit(&argc, argv);
      glutInitWindowSize(800, 600);
      glutInitWindowPosition(100, 100);
      glutCreateWindow("Ex08 - Two Independent VBOs in One Program");

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
