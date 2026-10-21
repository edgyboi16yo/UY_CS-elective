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
   * Example 12 - One VBO, Interleaved Position + Color
   * ------------------------------------------------------------
   * Concept: Position and color can share ONE buffer, interleaved as
   * [x,y,z,r,g,b, x,y,z,r,g,b, ...]. glVertexPointer/glColorPointer use
   * a non-zero stride and a small BYTE OFFSET (cast to a pointer) to
   * read their own fields from the same bound buffer.
   */

  GLuint VBOid;

  void interleavedTriangle() {
      GLfloat data[] = {
          // x,     y,     z,     r,    g,   b
          0.0f, 0.75f, 0.0f, 1.0f, 0.0f, 0.0f,
          -0.75f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
          0.75f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
      };
      GLsizei stride = 6 * sizeof(GLfloat);

       glGenBuffers(1, &VBOid);
       glBindBuffer(GL_ARRAY_BUFFER, VBOid);
       glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);





        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glVertexPointer(3, GL_FLOAT, stride, (void*)0);
        glColorPointer(3, GL_FLOAT, stride, (void*)(3 * sizeof(GLfloat)));

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &VBOid);
  }

  void display() {
      glClear(GL_COLOR_BUFFER_BIT);
      interleavedTriangle();
      glFlush();
  }

  int main(int argc, char** argv) {
      glutInit(&argc, argv);
      glutInitWindowSize(800, 600);
      glutInitWindowPosition(100, 100);
      glutCreateWindow("Ex12 - One Interleaved VBO (Position + Color)");

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
