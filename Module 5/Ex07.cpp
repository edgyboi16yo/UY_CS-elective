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
   * Example 07 - glBufferData Usage Hints
   * ------------------------------------------------------------
   * Concept: glBufferData's final argument is a USAGE HINT telling
   * OpenGL how the data will be used, so the driver can choose the best
   * memory location/strategy:
   *   GL_STATIC_DRAW - set once, drawn many times (most common for
   *                      unchanging geometry).
   *   GL_DYNAMIC_DRAW - changed repeatedly, drawn many times (used
   *                      later in this module for animated VBOs).
   *   GL_STREAM_DRAW - set once, drawn only a few times.
   * The RENDERED RESULT is identical either way - only the performance
   * characteristics differ. This example draws the same triangle three
   * times, each from a separately created VBO using a different hint,
   * purely to show the syntax side by side.
   */

  GLuint staticVBO, dynamicVBO, streamVBO;

  GLuint makeTriangleVBO(GLfloat offsetX, GLenum usage) {
      GLfloat verts[] = {
          offsetX + 0.0f, 0.75f, 0.0f,
          offsetX - 0.2f, 0.55f, 0.0f,




        offsetX + 0.2f, 0.55f, 0.0f
    };
    GLuint id;
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, verts, usage);
    return id;
}

void drawVBO(GLuint id) {
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    staticVBO = makeTriangleVBO(-0.6f, GL_STATIC_DRAW);
    dynamicVBO = makeTriangleVBO(0.0f, GL_DYNAMIC_DRAW);
    streamVBO = makeTriangleVBO(0.6f, GL_STREAM_DRAW);

    glColor3f(0.9f, 0.3f, 0.3f);
    drawVBO(staticVBO);
    glColor3f(0.3f, 0.9f, 0.3f);
    drawVBO(dynamicVBO);
    glColor3f(0.3f, 0.3f, 0.9f);
    drawVBO(streamVBO);

    glDeleteBuffers(1, &staticVBO);
    glDeleteBuffers(1, &dynamicVBO);
    glDeleteBuffers(1, &streamVBO);

    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Ex07 - glBufferData Usage Hints Compared");

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
