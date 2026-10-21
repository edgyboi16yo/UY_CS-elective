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
 * Example 09 - A Second Buffer for Indices (GL_ELEMENT_ARRAY_BUFFER)
 * ------------------------------------------------------------
 * Concept: Just as GL_ARRAY_BUFFER holds vertex data on the GPU,
 * GL_ELEMENT_ARRAY_BUFFER holds INDEX data on the GPU. The pattern is
 * identical: glGenBuffers, glBindBuffer, glBufferData - just with a
 * different target and a GLubyte index array. Once bound,
 * glDrawElements's final argument becomes an OFFSET (0) instead of a
 * real pointer, exactly like glVertexPointer's did for GL_ARRAY_BUFFER.
 */

GLuint VBOid, VBOindex;

void triangle() {
    GLfloat trianglevertices[] = {
        0.0f, .75f, 0.0f,
        -0.75f, 0.0f, 0.0f,
        0.75f, 0.0f, 0.0f
    };
    GLubyte indices[] = { 0, 1, 2 };

    glGenBuffers(1, &VBOid);
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, trianglevertices,
GL_STATIC_DRAW);

    glGenBuffers(1, &VBOindex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOindex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 3, indices, GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, 0);
    glDisableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBOid);
    glDeleteBuffers(1, &VBOindex);
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
      glutCreateWindow("Ex09 - glDrawElements()");

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
