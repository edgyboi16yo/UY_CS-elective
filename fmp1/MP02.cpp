#include <GL/glew.h>
#include <GL/freeglut.h>

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

GLuint vertexBuffer = 0;
GLuint indexBuffer = 0;

std::vector<GLfloat> vertices;
std::vector<GLuint> indices;

enum Feature
{
    BOUNDARY,
    LAWN,
    HORIZONTAL_PATH,
    VERTICAL_PATH,
    PLAYGROUND,
    GARDEN,
    SEATING,
    FEATURE_COUNT
};

struct ParkRectangle
{
    float x;
    float y;
    float width;
    float height;
};

const ParkRectangle layouts[2][FEATURE_COUNT] = {
    {
        { 40, 110, 560, 440 },   // Park boundary.
        { 50, 120, 540, 420 },   // Lawn.
        { 50, 310, 540, 40 },    // Horizontal path.
        { 300, 120, 40, 420 },   // Vertical path.
        { 80, 380, 170, 120 },   // Playground.
        { 390, 380, 160, 120 },  // Garden.
        { 390, 170, 160, 100 }   // Seating.
    },
    {
        { 40, 110, 560, 440 },
        { 50, 120, 540, 420 },
        { 50, 280, 540, 40 },
        { 250, 120, 40, 420 },
        { 340, 365, 210, 135 },
        { 80, 355, 130, 145 },
        { 350, 160, 190, 80 }
    }
};

const GLfloat featureColors[FEATURE_COUNT][3] = {
    { 0.75f, 0.78f, 0.80f },  // Boundary: gray.
    { 0.25f, 0.48f, 0.29f },  // Lawn: green.
    { 0.82f, 0.73f, 0.55f },  // Horizontal path: tan.
    { 0.82f, 0.73f, 0.55f },  // Vertical path: tan.
    { 0.95f, 0.56f, 0.18f },  // Playground: orange.
    { 0.65f, 0.35f, 0.85f },  // Garden: purple.
    { 0.20f, 0.62f, 0.92f }   // Seating: blue.
};

bool visible[3] = { true, true, true };
int currentLayout = 0;

std::string input;
std::string message = "Ready. Layout A selected.";

void addRectangle(const ParkRectangle& rectangle)
{
    GLuint firstVertex =
        static_cast<GLuint>(vertices.size() / 2);

    float x = rectangle.x;
    float y = rectangle.y;
    float right = x + rectangle.width;
    float top = y + rectangle.height;

    // Four unique corner vertices.
    const GLfloat positions[] = {
        x,     y,
        right, y,
        right, top,
        x,     top
    };

    vertices.insert(
        vertices.end(),
        positions,
        positions + 8
    );

    // Two triangles sharing the same corner vertices.
    const GLuint rectangleIndices[] = {
        firstVertex,
        firstVertex + 1,
        firstVertex + 2,

        firstVertex,
        firstVertex + 2,
        firstVertex + 3
    };

    indices.insert(
        indices.end(),
        rectangleIndices,
        rectangleIndices + 6
    );
}

void createBuffers()
{
    // Load both layouts once.
    for (int layout = 0; layout < 2; ++layout)
    {
        for (int feature = 0; feature < FEATURE_COUNT; ++feature)
        {
            addRectangle(layouts[layout][feature]);
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

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(GLuint),
        indices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void cleanup()
{
    if (vertexBuffer != 0)
    {
        glDeleteBuffers(1, &vertexBuffer);
        vertexBuffer = 0;
    }

    if (indexBuffer != 0)
    {
        glDeleteBuffers(1, &indexBuffer);
        indexBuffer = 0;
    }
}

void drawText(float x, float y, const std::string& text)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);

    for (unsigned char character : text)
    {
        glutBitmapCharacter(
            GLUT_BITMAP_HELVETICA_18,
            character
        );
    }
}

void drawFeature(int feature)
{
    std::size_t rectangleIndex =
        static_cast<std::size_t>(
            currentLayout * FEATURE_COUNT + feature
            );

    std::size_t byteOffset =
        rectangleIndex * 6 * sizeof(GLuint);

    glColor3fv(featureColors[feature]);

    glDrawElements(
        GL_TRIANGLES,
        6,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void*>(byteOffset)
    );
}

void processChoice()
{
    if (input.size() != 1 ||
        input[0] < '0' ||
        input[0] > '6')
    {
        message = "Invalid input. Enter a number from 0 to 6.";
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
        for (bool& featureVisible : visible)
        {
            featureVisible = true;
        }

        message = "All features are visible.";
        break;

    case 2:
        visible[0] = !visible[0];

        message = visible[0]
            ? "Playground shown."
            : "Playground hidden.";
        break;

    case 3:
        visible[1] = !visible[1];

        message = visible[1]
            ? "Garden shown."
            : "Garden hidden.";
        break;

    case 4:
        visible[2] = !visible[2];

        message = visible[2]
            ? "Seating shown."
            : "Seating hidden.";
        break;

    case 5:
        currentLayout = 0;
        message = "Layout A selected.";
        break;

    case 6:
        currentLayout = 1;
        message = "Layout B selected.";
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
    else if (key >= 32 &&
        key <= 126 &&
        input.size() < 20)
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
    glOrtho(0, 1100, 0, 650, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, nullptr);

    drawFeature(BOUNDARY);
    drawFeature(LAWN);
    drawFeature(HORIZONTAL_PATH);
    drawFeature(VERTICAL_PATH);

    for (int i = 0; i < 3; ++i)
    {
        if (visible[i])
        {
            drawFeature(PLAYGROUND + i);
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    drawText(
        40,
        610,
        "COMMUNITY PARK LAYOUT PLANNER"
    );

    drawText(
        40,
        575,
        currentLayout == 0 ? "Layout A" : "Layout B"
    );

    const char* names[] = {
        "Playground",
        "Garden",
        "Seating"
    };

    for (int i = 0; i < 3; ++i)
    {
        if (visible[i])
        {
            const ParkRectangle& rectangle =
                layouts[currentLayout][PLAYGROUND + i];

            drawText(
                rectangle.x + 10,
                rectangle.y + rectangle.height / 2,
                names[i]
            );
        }
    }

    drawText(650, 550, "MENU");
    drawText(650, 515, "1 - Show All");
    drawText(650, 480, "2 - Toggle Playground");
    drawText(650, 445, "3 - Toggle Garden");
    drawText(650, 410, "4 - Toggle Seating");
    drawText(650, 375, "5 - Layout A");
    drawText(650, 340, "6 - Layout B");
    drawText(650, 305, "0 - Exit");

    drawText(650, 255, "LEGEND");
    drawText(650, 225, "Orange: Playground | Purple: Garden");
    drawText(650, 195, "Blue: Seating | Tan: Walking path");
    drawText(650, 165, "Green: Lawn | Gray: Park boundary");

    drawText(650, 115, "Type a number, then press Enter.");
    drawText(650, 80, "Choice: " + input);

    drawText(40, 30, message);

    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1100, 650);

    glutCreateWindow(
        "MP02 - Community Park Layout Planner"
    );

    glutSetOption(
        GLUT_ACTION_ON_WINDOW_CLOSE,
        GLUT_ACTION_GLUTMAINLOOP_RETURNS
    );

    GLenum result = glewInit();

    if (result != GLEW_OK || !GLEW_VERSION_1_5)
    {
        std::cerr
            << "OpenGL 1.5 buffer support is required.\n";

        glutDestroyWindow(glutGetWindow());
        return EXIT_FAILURE;
    }

    glClearColor(0.08f, 0.10f, 0.14f, 1.0f);

    createBuffers();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutCloseFunc(cleanup);

    glutMainLoop();

    return EXIT_SUCCESS;
}