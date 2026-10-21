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
   * Example 15 - Proper VBO Cleanup on Exit (ESC Key)
   * ------------------------------------------------------------





* Concept: A VBO occupies GPU memory for as long as the buffer ID is
* alive. A well-behaved program releases that memory before exiting.
* Pressing ESC here disables the client state, unbinds the buffer,
* deletes it with glDeleteBuffers, and then exits - mirroring the
* lecture's monitorKeyboard() cleanup pattern.
*/

GLuint VBOid;

void triangle() {
    GLfloat trianglevertices[] = {
        0.0f, .75f, 0.0f,
        -0.75f, 0.0f, 0.0f,
        0.75f, 0.0f, 0.0f
    };

    if (VBOid == 0) glGenBuffers(1, &VBOid);
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, trianglevertices,
GL_STATIC_DRAW);

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

void monitorKeyboard(unsigned char key, int x, int y) {
    if (key == 27) {   // ESC
        // Disable client side capability for normal operation
        glDisableClientState(GL_VERTEX_ARRAY);

        // bind with 0, so switch back to normal pointer operation
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &VBOid);

        exit(0);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);





         glutCreateWindow("Ex15 - ESC-Key Cleanup and Exit");

         glutDisplayFunc(display);
         glutKeyboardFunc(monitorKeyboard);

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
