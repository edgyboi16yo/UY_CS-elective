#include <GL/glew.h>
#include <GL/freeglut.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr float kWarehouseLeft = -0.92f;
constexpr float kWarehouseRight = 0.92f;
constexpr float kWarehouseBottom = -0.82f;
constexpr float kWarehouseTop = 0.82f;
constexpr float kRobotHalfWidth = 0.075f;
constexpr float kRobotHalfHeight = 0.075f;

struct RobotVertex {
    GLfloat x;
    GLfloat y;
};

enum class CommandType {
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    SetStep,
    ReturnToStart,
    AutoMove,
    Exit
};

struct Command {
    CommandType type;
    float value = 0.0f;
};

GLuint robotVbo = 0;
std::vector<RobotVertex> robotVertices;
std::mutex commandMutex;
std::vector<Command> pendingCommands;
std::atomic<bool> running{true};
bool autoMoveEnabled = false;
bool stepInputMode = false;
std::string stepInput;
std::string uiMessage;
float robotX = 0.0f;
float robotY = 0.0f;
float stepSize = 0.12f;
int windowWidth = 760;
int windowHeight = 760;
int autoTargetIndex = 0;

struct AutoTarget {
    float x;
    float y;
    const char* label;
};

constexpr std::array<AutoTarget, 4> kAutoTargets = {{
    {-0.67f, 0.515f, "ZONE A"},
    {0.67f, 0.515f, "ZONE B"},
    {0.67f, -0.515f, "ZONE C"},
    {0.0f, 0.0f, "START"}
}};

void queueCommand(Command command) {
    std::lock_guard<std::mutex> lock(commandMutex);
    pendingCommands.push_back(command);
}

void printMenu() {
    std::cout << "\n=== Warehouse Robot Route Simulator ===\n"
              << "1. Move Left\n"
              << "2. Move Right\n"
              << "3. Move Up\n"
              << "4. Move Down\n"
              << "5. Set Step Size\n"
              << "6. Return to Start\n"
              << "7. Auto-Move Demo\n"
              << "0. Exit\n"
              << "Choice: " << std::flush;
}

void consoleInputLoop() {
    printMenu();
    int choice = -1;
    while (running && std::cin >> choice) {
        switch (choice) {
        case 1:
            queueCommand({CommandType::MoveLeft});
            break;
        case 2:
            queueCommand({CommandType::MoveRight});
            break;
        case 3:
            queueCommand({CommandType::MoveUp});
            break;
        case 4:
            queueCommand({CommandType::MoveDown});
            break;
        case 5: {
            float requested = 0.0f;
            std::cout << "\n--- STEP SIZE MODE ---\n"
                      << "You are now choosing the robot movement step size.\n"
                      << "Enter a positive decimal from 0.01 to 0.50.\n"
                      << "Step size: " << std::flush;
            if (!(std::cin >> requested)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid step size. Returning to the main menu.\n";
            } else if (requested <= 0.0f) {
                std::cout << "Step size must be positive.\n";
            } else {
                queueCommand({CommandType::SetStep, requested});
                std::cout << "Step size accepted. The robot will use " << requested
                          << " units per movement.\n";
            }
            break;
        }
        case 6:
            queueCommand({CommandType::ReturnToStart});
            break;
        case 7:
            queueCommand({CommandType::AutoMove});
            break;
        case 0:
            queueCommand({CommandType::Exit});
            running = false;
            return;
        default:
            std::cout << "Invalid choice. Select 0 through 7.\n";
            break;
        }
        if (running) {
            printMenu();
        }
    }
}

void drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (unsigned char character : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, character);
    }
}

void drawRectangle(float left, float bottom, float right, float top,
                   float red, float green, float blue) {
    glColor3f(red, green, blue);
    glBegin(GL_QUADS);
    glVertex2f(left, bottom);
    glVertex2f(right, bottom);
    glVertex2f(right, top);
    glVertex2f(left, top);
    glEnd();
}

void drawWarehouse() {
    glColor3f(0.16f, 0.18f, 0.22f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    // Simple warehouse floor grid.
    glColor3f(0.25f, 0.27f, 0.31f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (float x = -0.75f; x <= 0.75f; x += 0.15f) {
        glVertex2f(x, kWarehouseBottom + 0.03f);
        glVertex2f(x, kWarehouseTop - 0.03f);
    }
    for (float y = -0.70f; y <= 0.70f; y += 0.15f) {
        glVertex2f(kWarehouseLeft + 0.03f, y);
        glVertex2f(kWarehouseRight - 0.03f, y);
    }
    glEnd();

    glColor3f(0.78f, 0.80f, 0.84f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(kWarehouseLeft, kWarehouseBottom);
    glVertex2f(kWarehouseRight, kWarehouseBottom);
    glVertex2f(kWarehouseRight, kWarehouseTop);
    glVertex2f(kWarehouseLeft, kWarehouseTop);
    glEnd();

    // Mark the robot's starting location.
    glColor3f(0.95f, 0.78f, 0.15f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.13f, -0.13f);
    glVertex2f(0.13f, -0.13f);
    glVertex2f(0.13f, 0.13f);
    glVertex2f(-0.13f, 0.13f);
    glEnd();
    glColor3f(0.75f, 0.76f, 0.80f);
    drawText(-0.09f, -0.18f, "START");

    glColor3f(0.95f, 0.95f, 0.95f);
    drawText(-0.35f, 0.90f, "WAREHOUSE ROBOT ROUTE SIMULATOR");

    // Three simple color-coded destination zones.
    drawRectangle(-0.82f, 0.38f, -0.52f, 0.65f, 0.65f, 0.18f, 0.18f);
    drawRectangle(0.52f, 0.38f, 0.82f, 0.65f, 0.18f, 0.60f, 0.30f);
    drawRectangle(0.52f, -0.65f, 0.82f, -0.38f, 0.20f, 0.38f, 0.72f);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(-0.78f, 0.50f, "ZONE A");
    drawText(0.56f, 0.50f, "ZONE B");
    drawText(0.56f, -0.53f, "ZONE C");

    glColor3f(0.30f, 0.32f, 0.37f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (float x = -0.75f; x <= 0.75f; x += 0.25f) {
        glVertex2f(x, -0.20f);
        glVertex2f(x + 0.10f, -0.20f);
    }
    glEnd();
}

void drawRobot() {
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, robotVbo);
    glVertexPointer(2, GL_FLOAT, sizeof(RobotVertex), nullptr);
    glColor3f(1.0f, 0.78f, 0.10f);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(robotVertices.size()));
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glColor3f(0.12f, 0.08f, 0.02f);
    drawText(robotX - 0.045f, robotY - 0.01f, "R");
}

bool useRightPanelLayout() {
    return windowWidth > static_cast<int>(windowHeight * 1.35f);
}

float bottomPanelHeight() {
    return std::min(142.0f, std::max(112.0f, windowHeight * 0.34f));
}

float rightPanelWidth() {
    return std::min(350.0f, std::max(270.0f, windowWidth * 0.28f));
}

void setWarehouseViewport() {
    const bool useRightPanel = useRightPanelLayout();
    const float panelWidth = rightPanelWidth();
    const float panelHeight = bottomPanelHeight();
    int square = 1;
    int viewportX = 0;
    int viewportY = 0;

    if (useRightPanel) {
        square = std::max(1, static_cast<int>(std::min(
            static_cast<float>(windowHeight - 24),
            static_cast<float>(windowWidth) - panelWidth - 24.0f)));
        viewportX = 12;
        viewportY = (windowHeight - square) / 2;
    } else {
        square = std::max(1, static_cast<int>(std::min(
            static_cast<float>(windowWidth - 24),
            static_cast<float>(windowHeight) - panelHeight - 24.0f)));
        viewportX = (windowWidth - square) / 2;
        viewportY = static_cast<int>(panelHeight) + 12;
    }
    glViewport(viewportX, viewportY, square, square);
}

void drawControlPanel() {
    const bool useRightPanel = useRightPanelLayout();
    const float panelWidth = useRightPanel ? rightPanelWidth() : static_cast<float>(windowWidth - 24);
    const float panelHeight = useRightPanel ? static_cast<float>(windowHeight - 24) : bottomPanelHeight();
    const float panelX = useRightPanel ? static_cast<float>(windowWidth) - panelWidth - 12.0f : 12.0f;
    const float panelY = 12.0f;

    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, windowWidth, 0.0, windowHeight, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor4f(0.04f, 0.05f, 0.07f, 0.96f);
    glBegin(GL_QUADS);
    glVertex2f(panelX, panelY);
    glVertex2f(panelX + panelWidth, panelY);
    glVertex2f(panelX + panelWidth, panelY + panelHeight);
    glVertex2f(panelX, panelY + panelHeight);
    glEnd();

    glColor3f(0.95f, 0.78f, 0.15f);
    drawText(panelX + 14.0f, panelY + panelHeight - 24.0f, "ROBOT CONTROLS");
    glColor3f(0.88f, 0.90f, 0.94f);

    if (useRightPanel) {
        drawText(panelX + 14.0f, panelY + panelHeight - 52.0f, "1 / 2 / 3 / 4  Move");
        drawText(panelX + 14.0f, panelY + panelHeight - 76.0f, "6              Return to start");
        drawText(panelX + 14.0f, panelY + panelHeight - 100.0f, "7              Toggle auto-move");
        drawText(panelX + 14.0f, panelY + panelHeight - 124.0f, "0              Exit");
        drawText(panelX + 14.0f, panelY + panelHeight - 162.0f, "Console menu:");
        drawText(panelX + 14.0f, panelY + panelHeight - 186.0f, "5 sets the step size");
        drawText(panelX + 14.0f, panelY + panelHeight - 224.0f, "Step size:");
        drawText(panelX + 14.0f, panelY + panelHeight - 248.0f, std::to_string(stepSize).c_str());
        drawText(panelX + 14.0f, panelY + panelHeight - 286.0f, "Robot position:");
        drawText(panelX + 14.0f, panelY + panelHeight - 310.0f,
                 ("(" + std::to_string(robotX) + ", " + std::to_string(robotY) + ")").c_str());
        drawText(panelX + 14.0f, panelY + panelHeight - 348.0f,
                 autoMoveEnabled ? "Status: AUTO-MOVING" : "Status: READY");
    } else if (windowWidth < 560) {
        // Compact layout for narrow windows: two short rows plus status.
        drawText(panelX + 14.0f, panelY + panelHeight - 50.0f, "1-4 Move   6 Restart");
        drawText(panelX + 14.0f, panelY + panelHeight - 72.0f, "7 Auto   0 Exit   5 Step size");
        drawText(panelX + 14.0f, panelY + 32.0f,
                 stepInputMode ? "TYPE VALUE + ENTER" : (autoMoveEnabled ? "AUTO-MOVING" : "READY"));
    } else {
        drawText(panelX + 14.0f, panelY + panelHeight - 52.0f, "1/2/3/4 Move");
        drawText(panelX + 185.0f, panelY + panelHeight - 52.0f, "6 Return to start");
        drawText(panelX + 380.0f, panelY + panelHeight - 52.0f, "7 Auto-move");
        drawText(panelX + 14.0f, panelY + panelHeight - 82.0f, "0 Exit");
        drawText(panelX + 185.0f, panelY + panelHeight - 82.0f, "Console: 5 sets step size");
        drawText(panelX + 14.0f, panelY + 110.0f, "Current step size:");
        drawText(panelX + 145.0f, panelY + 110.0f, std::to_string(stepSize).c_str());
        drawText(panelX + 380.0f, panelY + 110.0f,
                 ("Position: " + std::to_string(robotX) + ", " + std::to_string(robotY)).c_str());
        drawText(panelX + 14.0f, panelY + 80.0f,
                 autoMoveEnabled ? "Status: AUTO-MOVING" : "Status: READY");
    }

    if (stepInputMode) {
        glColor3f(1.0f, 0.82f, 0.18f);
        const std::string valueText = "STEP SIZE: " + (stepInput.empty() ? std::string("_") : stepInput + "_");
        if (useRightPanel) {
            drawText(panelX + 14.0f, panelY + 48.0f, "Type a value, then press Enter");
            drawText(panelX + 14.0f, panelY + 26.0f, valueText);
        } else if (windowWidth >= 560) {
            drawText(panelX + 380.0f, panelY + 32.0f, valueText);
        } else {
            drawText(panelX + 145.0f, panelY + 32.0f, valueText);
        }
    }
    if (!uiMessage.empty() && !stepInputMode) {
        glColor3f(0.50f, 0.85f, 0.95f);
        drawText(panelX + 14.0f, panelY + 14.0f, uiMessage);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void updateRobotBuffer(float dx, float dy) {
    if (dx == 0.0f && dy == 0.0f) {
        return;
    }

    // Keep the full triangle inside the warehouse boundary.
    const float nextX = std::clamp(robotX + dx,
                                   kWarehouseLeft + kRobotHalfWidth,
                                   kWarehouseRight - kRobotHalfWidth);
    const float nextY = std::clamp(robotY + dy,
                                   kWarehouseBottom + kRobotHalfHeight,
                                   kWarehouseTop - kRobotHalfHeight);
    const float actualDx = nextX - robotX;
    const float actualDy = nextY - robotY;
    robotX = nextX;
    robotY = nextY;

    glBindBuffer(GL_ARRAY_BUFFER, robotVbo);
    void* mapped = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    if (mapped != nullptr) {
        auto* vertices = static_cast<RobotVertex*>(mapped);
        for (auto& vertex : robotVertices) {
            vertex.x += actualDx;
            vertex.y += actualDy;
        }
        for (std::size_t i = 0; i < robotVertices.size(); ++i) {
            vertices[i] = robotVertices[i];
        }
        if (!glUnmapBuffer(GL_ARRAY_BUFFER)) {
            std::cerr << "Warning: OpenGL reported a failed buffer unmap.\n";
        }
    } else {
        // Safe fallback for contexts where mapping is unavailable.
        for (auto& vertex : robotVertices) {
            vertex.x += actualDx;
            vertex.y += actualDy;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        static_cast<GLsizeiptr>(robotVertices.size() * sizeof(RobotVertex)),
                        robotVertices.data());
        std::cerr << "Warning: glMapBuffer() failed; used glBufferSubData() fallback.\n";
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void resetRobot() {
    const float dx = -robotX;
    const float dy = -robotY;
    updateRobotBuffer(dx, dy);
}

void autoMoveTimer(int) {
    if (!autoMoveEnabled || !running) {
        return;
    }
    const AutoTarget& target = kAutoTargets[autoTargetIndex];
    const float dx = target.x - robotX;
    const float dy = target.y - robotY;
    const float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= stepSize) {
        updateRobotBuffer(dx, dy);
        autoTargetIndex = (autoTargetIndex + 1) % static_cast<int>(kAutoTargets.size());
    } else {
        updateRobotBuffer((dx / distance) * stepSize,
                          (dy / distance) * stepSize);
    }
    glutPostRedisplay();
    // The next timer is scheduled here, rather than from display().
    glutTimerFunc(120, autoMoveTimer, 0);
}

void applyCommand(const Command& command) {
    switch (command.type) {
    case CommandType::MoveLeft:
        updateRobotBuffer(-stepSize, 0.0f);
        break;
    case CommandType::MoveRight:
        updateRobotBuffer(stepSize, 0.0f);
        break;
    case CommandType::MoveUp:
        updateRobotBuffer(0.0f, stepSize);
        break;
    case CommandType::MoveDown:
        updateRobotBuffer(0.0f, -stepSize);
        break;
    case CommandType::SetStep:
        if (command.value > 0.0f) {
            stepSize = std::min(command.value, 0.50f);
            std::cout << "Step size set to " << stepSize << ".\n";
        }
        break;
    case CommandType::ReturnToStart:
        autoMoveEnabled = false;
        resetRobot();
        std::cout << "Robot returned to start.\n";
        break;
    case CommandType::AutoMove:
        autoMoveEnabled = !autoMoveEnabled;
        if (autoMoveEnabled) {
            autoTargetIndex = 0;
            std::cout << "Auto-Move Demo started: Zone A -> Zone B -> Zone C -> Start.\n";
            glutTimerFunc(120, autoMoveTimer, 0);
        } else {
            std::cout << "Auto-Move Demo stopped.\n";
        }
        break;
    case CommandType::Exit:
        autoMoveEnabled = false;
        running = false;
        glutLeaveMainLoop();
        break;
    }
}

void processQueuedCommands(int) {
    std::vector<Command> commands;
    {
        std::lock_guard<std::mutex> lock(commandMutex);
        commands.swap(pendingCommands);
    }
    for (const Command& command : commands) {
        applyCommand(command);
    }
    glutPostRedisplay();
    if (running) {
        glutTimerFunc(50, processQueuedCommands, 0);
    }
}

void display() {
    glViewport(0, 0, windowWidth, windowHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    setWarehouseViewport();
    drawWarehouse();
    drawRobot();
    drawControlPanel();
    glutSwapBuffers();
}

void reshape(int width, int height) {
    windowWidth = std::max(1, width);
    windowHeight = std::max(1, height);
    setWarehouseViewport();
}

void keyboard(unsigned char key, int, int) {
    if (stepInputMode) {
        if (key == 13 || key == 10) {
            try {
                std::size_t parsedCharacters = 0;
                const float requested = std::stof(stepInput, &parsedCharacters);
                if (parsedCharacters != stepInput.size() || requested <= 0.0f || requested > 0.50f) {
                    uiMessage = "Use a positive value from 0.01 to 0.50.";
                } else {
                    stepSize = requested;
                    uiMessage = "Step size updated to " + std::to_string(stepSize).substr(0, 5);
                    stepInputMode = false;
                    stepInput.clear();
                }
            } catch (...) {
                uiMessage = "Invalid step size. Try a value from 0.01 to 0.50.";
            }
        } else if (key == 27) {
            stepInputMode = false;
            stepInput.clear();
            uiMessage = "Step size entry cancelled.";
        } else if (key == 8) {
            if (!stepInput.empty()) {
                stepInput.pop_back();
            }
        } else if ((key >= '0' && key <= '9') || (key == '.' && stepInput.find('.') == std::string::npos)) {
            stepInput.push_back(static_cast<char>(key));
        }
        glutPostRedisplay();
        return;
    }

    switch (key) {
    case '1': queueCommand({CommandType::MoveLeft}); break;
    case '2': queueCommand({CommandType::MoveRight}); break;
    case '3': queueCommand({CommandType::MoveUp}); break;
    case '4': queueCommand({CommandType::MoveDown}); break;
    case '5':
        stepInputMode = true;
        stepInput.clear();
        uiMessage.clear();
        break;
    case '6': queueCommand({CommandType::ReturnToStart}); break;
    case '7': queueCommand({CommandType::AutoMove}); break;
    case '0': queueCommand({CommandType::Exit}); break;
    default: return;
    }
    glutPostRedisplay();
}

void initializeRobot() {
    robotVertices = {
        {0.0f, 0.09f},
        {-0.09f, -0.075f},
        {0.09f, -0.075f}
    };
    glGenBuffers(1, &robotVbo);
    glBindBuffer(GL_ARRAY_BUFFER, robotVbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(robotVertices.size() * sizeof(RobotVertex)),
                 robotVertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void shutdown() {
    running = false;
    if (robotVbo != 0) {
        glDeleteBuffers(1, &robotVbo);
        robotVbo = 0;
    }
}

} // namespace

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(760, 760);
    glutCreateWindow("Warehouse Robot Route Simulator");
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    const GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "GLEW initialization failed: "
                  << glewGetErrorString(glewStatus) << "\n";
        return EXIT_FAILURE;
    }

    glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    initializeRobot();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(50, processQueuedCommands, 0);

    std::thread consoleThread(consoleInputLoop);
    glutMainLoop();

    running = false;
    if (consoleThread.joinable()) {
        consoleThread.join();
    }
    shutdown();
    return EXIT_SUCCESS;
}
