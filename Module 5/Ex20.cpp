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
   * Example 20 - Capstone: A Full VBO-Based, Animated, Cleaned-Up Scene
   * ------------------------------------------------------------
   * Concept: Combines every technique from this module into one
   * program:
   *   - A position VBO and a color VBO for a triangle (Example 11).
   *   - An element (index) VBO for a quad "ground" (Example 10).
   *   - Continuous glMapBuffer-driven animation via a self-rescheduling
   *     glutTimerFunc (Example 16).
   *   - Proper GPU memory cleanup on ESC (Example 15).
   */

  GLuint triPosVBO, triColorVBO, groundVBO, groundIndexVBO;
  bool ready = false;

  void buildScene() {
      GLfloat triPos[] = {
          0.0f, 0.75f, 0.0f,
          -0.3f, 0.35f, 0.0f,
          0.3f, 0.35f, 0.0f
      };
      GLfloat triColor[] = {




       1.0f, 0.2f, 0.2f,
       0.2f, 1.0f, 0.2f,
       0.2f, 0.2f, 1.0f
    };
    GLfloat groundVerts[] = {
        -0.9f, -0.9f, 0.0f, 0.9f, -0.9f, 0.0f, 0.9f, -0.6f, 0.0f,            -0.9f, -0.6f, 0.0f
    };
    GLubyte groundIndices[] = { 0, 1, 2, 0, 2, 3 };

    glGenBuffers(1, &triPosVBO);
    glBindBuffer(GL_ARRAY_BUFFER, triPosVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triPos), triPos, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &triColorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, triColorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triColor), triColor, GL_STATIC_DRAW);

    glGenBuffers(1, &groundVBO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVerts), groundVerts, GL_STATIC_DRAW);

    glGenBuffers(1, &groundIndexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundIndexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices,
GL_STATIC_DRAW);

    ready = true;
}

void drawGround() {
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundIndexVBO);
    glColor3f(0.3f, 0.6f, 0.3f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void drawTriangle() {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, triPosVBO);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, triColorVBO);
    glColorPointer(3, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}




void display() {
    if (!ready) buildScene();
    glClear(GL_COLOR_BUFFER_BIT);
    drawGround();
    drawTriangle();
    glFlush();
}

void animateStep(int value) {
    static float dy = -0.01f;

    glBindBuffer(GL_ARRAY_BUFFER, triPosVBO);
    GLfloat* mapped = (GLfloat*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    if (!mapped) { cerr << "Cannot map animation buffer." << endl; exit(1); }
    if (mapped[1] > 0.75f || mapped[1] < 0.4f) dy = -dy;
    mapped[1] += dy; mapped[4] += dy; mapped[7] += dy;
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glutPostRedisplay();
    glutTimerFunc(100, animateStep, 0);
}

void monitorKeyboard(unsigned char key, int x, int y) {
    if (key == 27) {   // ESC: release every GPU buffer before exiting
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &triPosVBO);
        glDeleteBuffers(1, &triColorVBO);
        glDeleteBuffers(1, &groundVBO);
        glDeleteBuffers(1, &groundIndexVBO);
        exit(0);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Ex20 - Capstone");

    glutDisplayFunc(display);
    glutKeyboardFunc(monitorKeyboard);
    glutTimerFunc(100, animateStep, 0);

#ifndef __APPLE__
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cerr << "GLEW initialization failed." << endl;
        return 1;
    }
    if (err == GLEW_OK) {
        buildScene(); // Create buffers before any timer can use them.
        glutMainLoop();
    }
#else
    glutMainLoop();




#endif
    return 0;
}
