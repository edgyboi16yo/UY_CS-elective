// CS0045 Module 5 - standalone exercise reference solution.
#include <GL/glew.h> // GLEW must come before FreeGLUT.
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

constexpr float PI = 3.14159265358979323846f;
GLuint positionVBO = 0, colorVBO = 0, indexVBO = 0, secondVBO = 0;

template<class T, std::size_t N>
void upload(GLuint& id, GLenum target, const T (&data)[N], GLenum usage) {
    glGenBuffers(1, &id);
    glBindBuffer(target, id);
    glBufferData(target, sizeof(data), data, usage);
    glBindBuffer(target, 0);
}

void draw(GLuint id, GLenum mode, GLsizei count, float r, float g, float b) {
    glColor3f(r, g, b);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, nullptr); // Byte offset into bound VBO.
    glDrawArrays(mode, 0, count);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawColored(GLenum mode, GLsizei count, bool interleaved, bool indexed) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    const GLsizei stride = interleaved ? 6 * sizeof(GLfloat) : 0;
    glVertexPointer(3, GL_FLOAT, stride, nullptr);
    if (interleaved) {
        glColorPointer(3, GL_FLOAT, stride,
                       reinterpret_cast<const void*>(3 * sizeof(GLfloat)));
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glColorPointer(3, GL_FLOAT, 0, nullptr);
    }
    if (indexed) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
        glDrawElements(mode, count, GL_UNSIGNED_BYTE, nullptr);
    } else {
        glDrawArrays(mode, 0, count);
    }
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void drawIndexed(GLuint positions, GLenum mode, GLsizei count,
                 float r, float g, float b) {
    glColor3f(r, g, b);
    glBindBuffer(GL_ARRAY_BUFFER, positions);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    glDrawElements(mode, count, GL_UNSIGNED_BYTE, nullptr);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLfloat* map(GLuint id) {
    glBindBuffer(GL_ARRAY_BUFFER, id);
    auto* data = static_cast<GLfloat*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    if (!data) {
        std::cerr << "glMapBuffer failed. Closing program.\n";
        std::exit(EXIT_FAILURE);
    }
    return data;
}

void unmap() {
    if (glUnmapBuffer(GL_ARRAY_BUFFER) != GL_TRUE) {
        std::cerr << "Buffer data became invalid. Closing program.\n";
        std::exit(EXIT_FAILURE);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void cleanup() {
    // Undo enabling the vertex array reader.
    glDisableClientState(GL_VERTEX_ARRAY);
    // Undo enabling the optional color array reader.
    glDisableClientState(GL_COLOR_ARRAY);
    // Undo binding a vertex/color buffer.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Undo binding an element/index buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // Undo generating each buffer ID: release all owned GPU storage.
    glDeleteBuffers(1, &positionVBO);
    glDeleteBuffers(1, &colorVBO);
    glDeleteBuffers(1, &indexVBO);
    glDeleteBuffers(1, &secondVBO);
    positionVBO = colorVBO = indexVBO = secondVBO = 0;
}

// Q14: Indexed pentagon with one center
void initialize() {
    GLfloat vertices[18] = {}; // Center at index 0, stored once.
    for (int i=0; i<5; ++i) {
        float a = PI/2 + 2*PI*i/5;
        vertices[(i+1)*3] = .65f*std::cos(a);
        vertices[(i+1)*3+1] = .65f*std::sin(a);
    }
    upload(positionVBO, GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW);
    const GLubyte indices[] = {0,1,2,3,4,5,1};
    upload(indexVBO, GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW);
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawIndexed(positionVBO, GL_TRIANGLE_FAN, 7, .7f,.3f,1);
    glutSwapBuffers();
}

void monitorKeyboard(unsigned char key, int, int) {
    if (key == 27) {
        cleanup();
        std::exit(EXIT_SUCCESS);
    }
    
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Q14 - Indexed pentagon with one center");
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed.\n";
        return EXIT_FAILURE;
    }
    if (!GLEW_VERSION_1_5) {
        std::cerr << "OpenGL 1.5 or later is required for VBOs.\n";
        return EXIT_FAILURE;
    }
    glClearColor(.06f, .07f, .1f, 1);
    initialize(); // Buffers exist BEFORE rendering or timer callbacks.
    glutDisplayFunc(display);
    glutKeyboardFunc(monitorKeyboard);
    glutCloseFunc(cleanup);
    
    glutMainLoop();
    return 0;
}
