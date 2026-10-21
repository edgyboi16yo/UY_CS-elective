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
 * Example 14 - glMapBuffer(): Editing VBO Data Already on the GPU
 * ------------------------------------------------------------
 * Concept: Once vertex data has been uploaded to the GPU, editing the
 * ORIGINAL client-side array no longer changes what is drawn - the
 * GPU has its own copy. glMapBuffer(target, access) hands back a
 * pointer that lets the application read/write that GPU-resident data
 * directly; glUnmapBuffer(target) must be called afterward to release
 * the mapping. Here, a single glMapBuffer call nudges the triangle's
 * three vertices to the right by a fixed amount, once, when the
 * program starts.
 */

GLuint VBOid;

void triangle() {
    GLfloat trianglevertices[] = {
        0.0f, .75f, 0.0f,
        -0.75f, 0.0f, 0.0f,
        0.75f, 0.0f, 0.0f
    };

    glGenBuffers(1, &VBOid);
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, trianglevertices,
GL_DYNAMIC_DRAW);

    // Map the buffer, nudge every vertex's X coordinate, then unmap
    GLfloat* mapped = (GLfloat*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    if (!mapped) { cerr << "Cannot map vertex buffer." << endl; exit(1); }
    mapped[0] += 0.1f; // vertex 0's x
    mapped[3] += 0.1f; // vertex 1's x
    mapped[6] += 0.1f; // vertex 2's x
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBOid);
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
      glutCreateWindow("Ex14 - glMapBuffer()");

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
