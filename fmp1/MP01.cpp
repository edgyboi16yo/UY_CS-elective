#include <GL/glew.h>
#include <GL/freeglut.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

GLuint vertexBuffer = 0;

std::vector<GLfloat> vertices;
GLint signalStart[3];
GLsizei signalVertexCount = 0;

int activeSignal = 0;
std::string input;
std::string message = "Ready. Type a choice, then press Enter.";

void addVertex(float x, float y)
{
    vertices.push_back(x);
    vertices.push_back(y);
}

void createGeometry()
{
    // Housing: two triangles.
    addVertex(170, 110);
    addVertex(350, 110);
    addVertex(350, 510);

    addVertex(170, 110);
    addVertex(350, 510);
    addVertex(170, 510);

    const int segments = 48;
    const float pi = 3.14159265358979323846f;
    const float centerY[] = { 430, 310, 190 };

    signalVertexCount = segments * 3;

    for (int light = 0; light < 3; ++light)
    {
        signalStart[light] =
            static_cast<GLint>(vertices.size() / 2);

        for (int i = 0; i < segments; ++i)
        {
            float angle1 = 2.0f * pi * i / segments;
            float angle2 = 2.0f * pi * (i + 1) / segments;

            addVertex(260, centerY[light]);

            addVertex(
                260 + 44 * std::cos(angle1),
                centerY[light] + 44 * std::sin(angle1)
            );

            addVertex(
                260 + 44 * std::cos(angle2),
                centerY[light] + 44 * std::sin(angle2)
            );
        }
    }

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(GLfloat),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawText(float x, float y, const std::string& text)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);

    for (unsigned char character : text)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, character);
    }
}

void cleanup()
{
    if (vertexBuffer != 0)
    {
        glDeleteBuffers(1, &vertexBuffer);
        vertexBuffer = 0;
    }
}

void processChoice()
{
    if (input.size() != 1 || input[0] < '0' || input[0] > '5')
    {
        message = "Invalid input. Enter a number from 0 to 5.";
        input.clear();
        glutPostRedisplay();
        return;
    }

    int choice = input[0] - '0';
    input.clear();

    switch (choice)
    {
    case 0:
        glutLeaveMainLoop();
        return;

    case 1:
        activeSignal = 0;
        message = "Red / Stop selected.";
        break;

    case 2:
        activeSignal = 1;
        message = "Yellow / Caution selected.";
        break;

    case 3:
        activeSignal = 2;
        message = "Green / Go selected.";
        break;

    case 4:
        activeSignal = -1;
        message = "All signals turned off.";
        break;

    case 5:
        activeSignal = 0;
        message = "Reset complete: Red / Stop.";
        break;
    }

    glutPostRedisplay();
}

void keyboard(unsigned char key, int, int)
{
    if (key == '\r' || key == '\n')
    {
        processChoice();
        return;
    }

    if (key == '\b' || key == 127)
    {
        if (!input.empty())
        {
            input.pop_back();
        }
    }
    else if (key >= 32 && key <= 126 && input.size() < 20)
    {
        input += static_cast<char>(key);
    }

    glutPostRedisplay();
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1000, 0, 620, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, nullptr);

    // Draw housing using its actual six vertices.
    glColor3f(0.15f, 0.16f, 0.18f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    const GLfloat colors[3][3] = {
        { 1.0f, 0.10f, 0.10f },
        { 1.0f, 0.85f, 0.05f },
        { 0.10f, 0.95f, 0.25f }
    };

    for (int i = 0; i < 3; ++i)
    {
        if (i == activeSignal)
        {
            glColor3fv(colors[i]);
        }
        else
        {
            glColor3f(0.07f, 0.07f, 0.08f);
        }

        glDrawArrays(
            GL_TRIANGLES,
            signalStart[i],
            signalVertexCount
        );
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    drawText(40, 575, "CAMPUS TRAFFIC SIGNAL VISUALIZER");

    drawText(510, 500, "MENU");
    drawText(510, 460, "1 - Red / Stop");
    drawText(510, 425, "2 - Yellow / Caution");
    drawText(510, 390, "3 - Green / Go");
    drawText(510, 355, "4 - All Off");
    drawText(510, 320, "5 - Reset");
    drawText(510, 285, "0 - Exit");

    drawText(510, 220, "Type a number, then press Enter.");
    drawText(510, 180, "Choice: " + input);

    const char* states[] = { "RED / STOP", "YELLOW / CAUTION", "GREEN / GO" };

    std::string state =
        activeSignal == -1 ? "ALL OFF" : states[activeSignal];

    drawText(170, 70, state);
    drawText(40, 25, message);

    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 620);

    glutCreateWindow("MP01 - Campus Traffic Signal Visualizer");

    glutSetOption(
        GLUT_ACTION_ON_WINDOW_CLOSE,
        GLUT_ACTION_GLUTMAINLOOP_RETURNS
    );

    GLenum result = glewInit();

    if (result != GLEW_OK || !GLEW_VERSION_1_5)
    {
        std::cerr << "OpenGL 1.5 buffer support is required.\n";
        glutDestroyWindow(glutGetWindow());
        return EXIT_FAILURE;
    }

    glClearColor(0.08f, 0.10f, 0.14f, 1.0f);

    createGeometry();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutCloseFunc(cleanup);

    glutMainLoop();

    return EXIT_SUCCESS;
}