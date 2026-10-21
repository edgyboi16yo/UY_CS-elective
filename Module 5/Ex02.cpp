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
   * Example 02 - The Same Triangle, as a Vertex Buffer Object (VBO)
   * ------------------------------------------------------------
   * Concept: The 5-step VBO pattern:
   *   1. Initialize the vertex data in an ordinary array.
   *   2. Generate a buffer ID and bind it: glGenBuffers, glBindBuffer,
   *      glBufferData - this UPLOADS the array to the GPU's own memory.
   *   3. glEnableClientState(GL_VERTEX_ARRAY).
   *   4. glVertexPointer(...) + glDrawArrays(...) - note the pointer
   *      argument is now an OFFSET (0), not a real pointer, because the
   *      data already lives in the currently BOUND buffer.
   *   5. Clean up: disable state, unbind, and delete the buffer.
   */

  GLuint VBOid;




void triangle() {
    // 1. initialize the triangle using an array
    GLfloat trianglevertices[] = {
        0.0f, .75f, 0.0f,
        -0.75f, 0.0f, 0.0f,
        0.75f, 0.0f, 0.0f
    };

    // 2. generate a Vertex Buffer Object ID and bind it
    glGenBuffers(1, &VBOid);
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, trianglevertices,
GL_STATIC_DRAW);

    // 3. activate and specify pointer to vertex array
    glEnableClientState(GL_VERTEX_ARRAY);

    // 4. draw the primitive object (0 = "use the currently bound buffer")
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 5. disable client side capability for normal operation
    glDisableClientState(GL_VERTEX_ARRAY);

    // bind with 0, so switch back to normal pointer operation
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
    glutCreateWindow("Ex02 - The Same Triangle, as a Vertex Buffer Object");

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
