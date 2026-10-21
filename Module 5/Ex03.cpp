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
   * Example 03 - GL_POINTS as a VBO
   * ------------------------------------------------------------
   * Concept: The same 5-step pattern applied to GL_POINTS.
   */

  GLuint VBOid;

  void points() {
      glPointSize(15.0f);

      GLfloat pointVertices[] = {
          -0.75f, -0.75f, 0.0f,
          -0.75f, 0.75f, 0.0f,
           0.75f, 0.75f, 0.0f,
           0.75f, -0.75f, 0.0f
      };

      glGenBuffers(1, &VBOid);
      glBindBuffer(GL_ARRAY_BUFFER, VBOid);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, pointVertices, GL_STATIC_DRAW);

      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, 0);
      glDrawArrays(GL_POINTS, 0, 4);




       glDisableClientState(GL_VERTEX_ARRAY);

       glBindBuffer(GL_ARRAY_BUFFER, 0);
       glDeleteBuffers(1, &VBOid);
  }

  void display() {
      glClear(GL_COLOR_BUFFER_BIT);
      glColor3f(1.0f, 1.0f, 1.0f);
      points();
      glFlush();
  }

  int main(int argc, char** argv) {
      glutInit(&argc, argv);
      glutInitWindowSize(800, 600);
      glutInitWindowPosition(100, 100);
      glutCreateWindow("Ex03 - GL_POINTS From a VBO");

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
