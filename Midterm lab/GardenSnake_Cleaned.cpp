#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glut.h>

// Standard library dependencies
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <string>

// ========================================
// DATA STRUCTURES
// ========================================
struct Cell {
    int x;
    int y;
};

struct Thorn {
    Cell cell;
    int movesLeft;
    bool pending;
    bool active;
};

struct Direction {
    int x;
    int y;
};

struct DifficultySettings {
    const char* name;
    int stepMs;
    int moveRockEveryMs;
    int addRockEveryMs;
    int waterDecayEveryMs;
    int startRocks;
    int maxRocks;
    int flowersToWin;
    int rockWarningMs;
    int dropLifetimeMoves;
};

struct RockWarning {
    bool active;
    int sourceIndex; // -1 means this warning will add a new rock.
    Cell target;
    int remainingMs;
};

// ========================================
// CONSTANTS AND DIFFICULTY SETTINGS
// ========================================
const int GRID_CELLS = 20;
const int MAX_SPAWN_ATTEMPTS = 200;
const int MAX_LAYOUT_ATTEMPTS = 100;
const int DROP_SEARCH_RADIUS = 6;
const int TILE_PIXELS = 30;
const int BOARD_PIXELS = GRID_CELLS * TILE_PIXELS;
const int WINDOW_WIDTH_PIXELS = 840;
const int WINDOW_HEIGHT_PIXELS = 600;
const int BED_COUNT = 4;
const int MAX_ROCKS = 11;
const int MAX_SNAKE = GRID_CELLS * GRID_CELLS;
const int WATER_FOR_FLOWER = 3;
const int PLANT_RANGE_CELLS = 2;
const int THORN_GROWTH_MOVES = 6;
const int TURN_BUFFER_SIZE = 2;
const int MIN_DELAY_MS = 70;
const int MAX_DELAY_MS = 250;
const int SPEED_STEP_MS = 20;
const int CLOCK_TICK_MS = 25;
const int SPEED_BUTTON_Y = 88;
const int SPEED_BUTTON_WIDTH = 92;
const int SPEED_BUTTON_HEIGHT = 27;
const int SLOWER_BUTTON_X = 622;
const int FASTER_BUTTON_X = 726;

const DifficultySettings DIFFICULTIES[3] = {
    {"EASY", 200, 10000, 15000, 0, 3, 5, 2, 2000, 40},
    {"MEDIUM", 150, 5000, 10000, 10000, 5, 8, 3, 1500, 35},
    {"HARD", 100, 4000, 7000, 6000, 7, 11, 4, 1000, 30}
};

// ========================================
// GAME STATE AND VARIABLES
// ========================================
Cell beds[BED_COUNT];
Cell rocks[MAX_ROCKS];
int rockCount = 0;
int difficultyIndex = 1;
RockWarning movingRock = {false, -1, {0, 0}, 0};
RockWarning addedRock = {false, -1, {0, 0}, 0};

enum GameState { MENU, PLAYING, PAUSED, WON, LOST };

GameState state = MENU;
Cell snake[MAX_SNAKE];
int snakeLength = 3;
int directionX = 1;
int directionY = 0;
Direction queuedTurns[TURN_BUFFER_SIZE];
int turnCount = 0;
int water = 0;
int totalDrops = 0;
int flowers = 0;
bool planted[BED_COUNT] = {};
Thorn thorns[BED_COUNT];
Cell drop = {0, 0};
bool dropVisible = false;
int dropMovesLeft = 0;
bool growOnNextMove = false;
int moveDelay = 150;
int lastClockMs = 0;
int stepClockMs = 0;
int moveRockClockMs = 0;
int addRockClockMs = 0;
int waterDecayClockMs = 0;
int windowWidth = WINDOW_WIDTH_PIXELS;
int windowHeight = WINDOW_HEIGHT_PIXELS;
std::string statusText = "Collect water drops";

// OpenGL reads these four vertices directly through glVertexPointer.
const GLfloat unitSquare[8] = {0, 0, 1, 0, 1, 1, 0, 1};

// ========================================
// BOARD HELPERS AND COLLISION CHECKS
// ========================================
bool sameCell(Cell a, Cell b) {
    return a.x == b.x && a.y == b.y;
}

const DifficultySettings& settings() {
    return DIFFICULTIES[difficultyIndex];
}

bool onBoard(Cell cell) {
    return cell.x >= 0 && cell.x < GRID_CELLS &&
           cell.y >= 0 && cell.y < GRID_CELLS;
}

// A planted bed must stay clear of turns already queued by the player.
bool bedInPlannedPath(Cell bed) {
    Cell position = snake[0];
    int x = directionX;
    int y = directionY;
    int steps = turnCount > 0 ? turnCount : 1;
    for (int i = 0; i < steps; ++i) {
        if (i < turnCount) {
            x = queuedTurns[i].x;
            y = queuedTurns[i].y;
        }
        position.x += x;
        position.y += y;
        if (sameCell(position, bed)) {
            return true;
        }
    }
    return false;
}

bool hasRock(Cell cell) {
    for (int i = 0; i < rockCount; ++i) {
        if (sameCell(cell, rocks[i])) {
            return true;
        }
    }
    return false;
}

bool reservedForRock(Cell cell) {
    return (movingRock.active && sameCell(cell, movingRock.target)) ||
           (addedRock.active && sameCell(cell, addedRock.target));
}

bool hasBed(Cell cell) {
    for (int i = 0; i < BED_COUNT; ++i) {
        if (sameCell(cell, beds[i])) {
            return true;
        }
    }
    return false;
}

bool hasFlower(Cell cell) {
    for (int i = 0; i < BED_COUNT; ++i) {
        if (planted[i] && sameCell(cell, beds[i])) {
            return true;
        }
    }
    return false;
}

bool hasSnake(Cell cell) {
    for (int i = 0; i < snakeLength; ++i) {
        if (sameCell(cell, snake[i])) {
            return true;
        }
    }
    return false;
}

int distanceTo(Cell a, Cell b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool plantingKeepsRoutesOpen(int index);

bool canPlantBed(int index) {
    return state == PLAYING && index >= 0 && index < BED_COUNT &&
           !planted[index] && water >= WATER_FOR_FLOWER &&
           !hasSnake(beds[index]) &&
           distanceTo(snake[0], beds[index]) <= PLANT_RANGE_CELLS &&
           !bedInPlannedPath(beds[index]) &&
           plantingKeepsRoutesOpen(index);
}

bool hasThorn(Cell cell) {
    for (int i = 0; i < BED_COUNT; ++i) {
        if (thorns[i].active && sameCell(cell, thorns[i].cell)) {
            return true;
        }
    }
    return false;
}

bool reservedForThorn(Cell cell) {
    for (int i = 0; i < BED_COUNT; ++i) {
        if ((thorns[i].pending || thorns[i].active) &&
            sameCell(cell, thorns[i].cell)) return true;
    }
    return false;
}

bool freeForDrop(Cell cell) {
    return !hasSnake(cell) && !hasRock(cell) && !hasBed(cell) &&
           !reservedForThorn(cell) && !reservedForRock(cell);
}

// Build the board after all warned rock changes. The original moving rock
// remains in rocks[] and solid until its warning actually finishes.
// ========================================
// PATHFINDING AND SAFETY VALIDATION
// ========================================
int projectedRocks(Cell result[MAX_ROCKS], const RockWarning& move,
                   const RockWarning& add) {
    int count = rockCount;
    for (int i = 0; i < count; ++i) {
        result[i] = rocks[i];
    }
    if (move.active) {
        result[move.sourceIndex] = move.target;
    }
    if (add.active && count < MAX_ROCKS) {
        result[count++] = add.target;
    }
    return count;
}

// The body will keep moving so it is not a permanent wall for route checks.
bool reachableWithRocks(const Cell testRocks[MAX_ROCKS], int count,
                        Cell extraThorn, bool useExtraThorn,
                        bool visited[GRID_CELLS][GRID_CELLS]) {
    if (!onBoard(snake[0])) {
        return false;
    }
    bool blocked[GRID_CELLS][GRID_CELLS] = {};
    for (int i = 0; i < count; ++i) {
        if (!onBoard(testRocks[i])) {
            return false;
        }
        blocked[testRocks[i].y][testRocks[i].x] = true;
        if (sameCell(testRocks[i], snake[0])) {
            return false;
        }
    }
    for (int i = 0; i < BED_COUNT; ++i) {
        if (planted[i]) {
            blocked[beds[i].y][beds[i].x] = true;
            if (sameCell(beds[i], snake[0])) {
                return false;
            }
        }
        if (thorns[i].pending || thorns[i].active) {
            blocked[thorns[i].cell.y][thorns[i].cell.x] = true;
            if (thorns[i].active && sameCell(thorns[i].cell, snake[0]))
                return false;
        }
    }
    if (useExtraThorn) {
        blocked[extraThorn.y][extraThorn.x] = true;
        if (sameCell(extraThorn, snake[0])) {
            return false;
        }
    }
    // A pending thorn is still passable and cannot become solid beneath the
    // snake, so a route may start there even though future paths avoid it.

    Cell queue[GRID_CELLS * GRID_CELLS];
    int queueFront = 0;
    int queueBack = 0;
    queue[queueBack++] = snake[0];
    visited[snake[0].y][snake[0].x] = true;

    const int stepX[4] = {1, -1, 0, 0};
    const int stepY[4] = {0, 0, 1, -1};
    while (queueFront < queueBack) {
        Cell current = queue[queueFront++];
        for (int i = 0; i < 4; ++i) {
            Cell next = {current.x + stepX[i], current.y + stepY[i]};
            if (!onBoard(next) || visited[next.y][next.x] ||
                blocked[next.y][next.x]) continue;
            visited[next.y][next.x] = true;
            queue[queueBack++] = next;
        }
    }

    if (dropVisible && !visited[drop.y][drop.x]) {
        return false;
    }

    for (int i = 0; i < BED_COUNT; ++i) {
        if (planted[i]) {
            continue;
        }
        bool canPlant = false;
        for (int y = 0; y < GRID_CELLS && !canPlant; ++y) {
            for (int x = 0; x < GRID_CELLS; ++x) {
                if (visited[y][x] &&
                    distanceTo({x, y}, beds[i]) <= PLANT_RANGE_CELLS) {
                    canPlant = true;
                    break;
                }
            }
        }
        if (!canPlant) {
            return false;
        }
    }
    return true;
}

bool bedsRemainReachable(Cell proposedThorn) {
    Cell futureRocks[MAX_ROCKS];
    int count = projectedRocks(futureRocks, movingRock, addedRock);
    bool visited[GRID_CELLS][GRID_CELLS] = {};
    return reachableWithRocks(futureRocks, count, proposedThorn, true, visited);
}

// ========================================
// THORN AND ROCK MANAGEMENT
// ========================================
bool markNewThorn(int bedIndex) {
    const int aroundX[4] = {1, -1, 0, 0};
    const int aroundY[4] = {0, 0, 1, -1};
    // Prefer an empty square; a body square is acceptable as a last resort
    // because activation waits until the entire snake has left it.
    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 0; i < 4; ++i) {
            Cell candidate = {beds[bedIndex].x + aroundX[i],
                              beds[bedIndex].y + aroundY[i]};
            if (candidate.x < 0 || candidate.x >= GRID_CELLS ||
                candidate.y < 0 || candidate.y >= GRID_CELLS) continue;
            if ((pass == 0 && hasSnake(candidate)) || hasRock(candidate) ||
                hasBed(candidate) || reservedForThorn(candidate) ||
                reservedForRock(candidate) ||
                (dropVisible && sameCell(candidate, drop))) continue;
            if (!bedsRemainReachable(candidate)) {
                continue;
            }
            thorns[bedIndex].cell = candidate;
            thorns[bedIndex].movesLeft = THORN_GROWTH_MOVES;
            thorns[bedIndex].pending = true;
            return true;
        }
    }
    return false;
}

void advanceThorns() {
    for (int i = 0; i < BED_COUNT; ++i) {
        if (!thorns[i].pending) {
            continue;
        }
        if (thorns[i].movesLeft > 0) --thorns[i].movesLeft;
        if (thorns[i].movesLeft == 0) {
            // A warning stays on the tile if the snake occupies it at the
            // deadline. It becomes solid only after the snake leaves.
            if (hasSnake(thorns[i].cell) ||
                (dropVisible && sameCell(thorns[i].cell, drop))) continue;
            thorns[i].pending = false;
            thorns[i].active = true;
            statusText = "A thorn has grown!";
        }
    }
}

void spawnDrop();

bool openForRockWarning(Cell target) {
    return target.x > 0 && target.x < GRID_CELLS - 1 &&
           target.y > 0 && target.y < GRID_CELLS - 1 &&
           !hasRock(target) && !hasBed(target) && !hasSnake(target) &&
           !reservedForThorn(target) && !reservedForRock(target) &&
           !bedInPlannedPath(target) &&
           !(dropVisible && sameCell(target, drop));
}

bool safeRockTarget(Cell target) {
    return onBoard(target) && !hasRock(target) && !hasBed(target) &&
           !hasSnake(target) && !reservedForThorn(target) &&
           !bedInPlannedPath(target) &&
           !(dropVisible && sameCell(target, drop));
}

bool safeRockLayout(const RockWarning& move, const RockWarning& add) {
    if (move.active &&
        (move.sourceIndex < 0 || move.sourceIndex >= rockCount ||
         !safeRockTarget(move.target))) return false;
    if (add.active &&
        (rockCount >= MAX_ROCKS || !safeRockTarget(add.target))) return false;
    if (move.active && add.active && sameCell(move.target, add.target))
        return false;

    Cell future[MAX_ROCKS];
    int count = projectedRocks(future, move, add);
    for (int i = 0; i < count; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (sameCell(future[i], future[j])) {
                return false;
            }
        }
    }
    bool visited[GRID_CELLS][GRID_CELLS] = {};
    return reachableWithRocks(future, count, {0, 0}, false, visited);
}

// Check both possible orders because move and add warnings may end separately.
bool safeRockPlans(const RockWarning& move, const RockWarning& add) {
    RockWarning noWarning = {false, -1, {0, 0}, 0};
    if (move.active && !safeRockLayout(move, noWarning)) {
        return false;
    }
    if (add.active && !safeRockLayout(noWarning, add)) {
        return false;
    }
    return safeRockLayout(move, add);
}

bool plantingKeepsRoutesOpen(int index) {
    if (flowers + 1 >= settings().flowersToWin) {
        return true;
    }

    planted[index] = true;
    RockWarning noWarning = {false, -1, {0, 0}, 0};
    Cell currentRocks[MAX_ROCKS];
    int count = projectedRocks(currentRocks, noWarning, noWarning);
    bool visited[GRID_CELLS][GRID_CELLS] = {};
    bool safe = reachableWithRocks(currentRocks, count, {0, 0}, false, visited);
    if (safe && (movingRock.active || addedRock.active)) {
        Cell futureRocks[MAX_ROCKS];
        count = projectedRocks(futureRocks, movingRock, addedRock);
        bool futureVisited[GRID_CELLS][GRID_CELLS] = {};
        safe = reachableWithRocks(futureRocks, count, {0, 0}, false,
                                  futureVisited);
    }
    planted[index] = false;
    return safe;
}

// Select both targets before showing either warning when two events are due.
bool planRockWarnings(bool needMove, bool needAdd) {
    const int stepX[4] = {1, -1, 0, 0};
    const int stepY[4] = {0, 0, 1, -1};
    int moveOptions = needMove ? rockCount * 4 : 1;
    int moveStart = needMove ? std::rand() % moveOptions : 0;
    int addOptions = needAdd ? GRID_CELLS * GRID_CELLS : 1;
    int addStart = needAdd ? std::rand() % addOptions : 0;

    for (int m = 0; m < moveOptions; ++m) {
        RockWarning moveWarning = movingRock;
        if (needMove) {
            int choice = (moveStart + m) % moveOptions;
            int index = choice / 4;
            int side = choice % 4;
            Cell target = {rocks[index].x + stepX[side],
                           rocks[index].y + stepY[side]};
            if (!openForRockWarning(target)) {
                continue;
            }
            moveWarning = {true, index, target, settings().rockWarningMs};
        }

        for (int a = 0; a < addOptions; ++a) {
            RockWarning addWarning = addedRock;
            if (needAdd) {
                int choice = (addStart + a) % addOptions;
                Cell target = {choice % GRID_CELLS, choice / GRID_CELLS};
                if (!openForRockWarning(target)) {
                    continue;
                }
                addWarning = {true, -1, target, settings().rockWarningMs};
            }
            if (!safeRockPlans(moveWarning, addWarning)) {
                continue;
            }
            if (needMove) {
                movingRock = moveWarning;
                moveRockClockMs = 0;
            }
            if (needAdd) {
                addedRock = addWarning;
                addRockClockMs = 0;
            }
            statusText = needMove && needAdd ? "Two rock warnings!" :
                         needMove ? "Rock moving soon!" : "New rock soon!";
            return true;
        }
    }
    // No fair place exists right now. Wait for the next interval.
    if (needMove) {
        moveRockClockMs = 0;
    }
    if (needAdd) {
        addRockClockMs = 0;
    }
    return false;
}

void finishRockWarnings(int elapsedMs) {
    if (movingRock.active) movingRock.remainingMs -= elapsedMs;
    if (addedRock.active) addedRock.remainingMs -= elapsedMs;
    bool finishMove = movingRock.active && movingRock.remainingMs <= 0;
    bool finishAdd = addedRock.active && addedRock.remainingMs <= 0;
    if (!finishMove && !finishAdd) {
        return;
    }

    RockWarning noWarning = {false, -1, {0, 0}, 0};
    if (finishMove && finishAdd && safeRockPlans(movingRock, addedRock)) {
        rocks[movingRock.sourceIndex] = movingRock.target;
        rocks[rockCount++] = addedRock.target;
        movingRock.active = addedRock.active = false;
        statusText = "Rocks changed!";
        return;
    }

    if (finishMove) {
        RockWarning remainingAdd = finishAdd ? noWarning : addedRock;
        if (safeRockPlans(movingRock, remainingAdd)) {
            rocks[movingRock.sourceIndex] = movingRock.target;
            statusText = "Rock moved!";
        } else {
            statusText = "Unsafe rock move skipped";
        }
        movingRock.active = false;
    }
    if (finishAdd) {
        RockWarning remainingMove = finishMove ? noWarning : movingRock;
        if (safeRockPlans(remainingMove, addedRock)) {
            rocks[rockCount++] = addedRock.target;
            statusText = "A rock appeared!";
        } else {
            statusText = "Unsafe new rock skipped";
        }
        addedRock.active = false;
    }
}

void advanceGameClocks(int elapsedMs) {
    finishRockWarnings(elapsedMs);
    moveRockClockMs += elapsedMs;
    if (rockCount + (addedRock.active ? 1 : 0) < settings().maxRocks)
        addRockClockMs += elapsedMs;

    if (settings().waterDecayEveryMs > 0 && water > 0) {
        waterDecayClockMs += elapsedMs;
        if (waterDecayClockMs >= settings().waterDecayEveryMs) {
            --water;
            waterDecayClockMs = 0;
            if (!dropVisible) spawnDrop();
            statusText = "Stored water dried up";
        }
    } else {
        waterDecayClockMs = 0;
    }

    bool needMove = !movingRock.active &&
                    moveRockClockMs >= settings().moveRockEveryMs;
    bool needAdd = !addedRock.active && rockCount < settings().maxRocks &&
                   addRockClockMs >= settings().addRockEveryMs;
    if (needMove || needAdd) planRockWarnings(needMove, needAdd);
}

// ========================================
// RANDOM SPAWNING AND GAME SETUP
// ========================================
void spawnDrop() {
    dropVisible = false;
    dropMovesLeft = 0;
    if (water == WATER_FOR_FLOWER) {
        return;
    }

    Cell futureRocks[MAX_ROCKS];
    int futureCount = projectedRocks(futureRocks, movingRock, addedRock);
    bool reachable[GRID_CELLS][GRID_CELLS] = {};
    if (!reachableWithRocks(futureRocks, futureCount, {0, 0}, false, reachable)) {
        // A snake standing on a warned tile will cancel that change later.
        RockWarning noWarning = {false, -1, {0, 0}, 0};
        futureCount = projectedRocks(futureRocks, noWarning, noWarning);
        bool currentReachable[GRID_CELLS][GRID_CELLS] = {};
        reachableWithRocks(futureRocks, futureCount, {0, 0}, false,
                           currentReachable);
        for (int y = 0; y < GRID_CELLS; ++y) {
            for (int x = 0; x < GRID_CELLS; ++x)
                reachable[y][x] = currentReachable[y][x];
        }
    }

    // Nearby water keeps the game manageable but still varies each time.
    for (int tries = 0; tries < MAX_SPAWN_ATTEMPTS; ++tries) {
        int offsetX = std::rand() % (2 * DROP_SEARCH_RADIUS + 1) - DROP_SEARCH_RADIUS;
        int offsetY = std::rand() % (2 * DROP_SEARCH_RADIUS + 1) - DROP_SEARCH_RADIUS;
        if (std::abs(offsetX) + std::abs(offsetY) > DROP_SEARCH_RADIUS) {
            continue;
        }
        Cell candidate = {snake[0].x + offsetX, snake[0].y + offsetY};
        if (candidate.x < 0 || candidate.x >= GRID_CELLS ||
            candidate.y < 0 || candidate.y >= GRID_CELLS) continue;
        if (freeForDrop(candidate) && reachable[candidate.y][candidate.x]) {
            drop = candidate;
            dropVisible = true;
            dropMovesLeft = settings().dropLifetimeMoves;
            return;
        }
    }

    for (int tries = 0; tries < MAX_SPAWN_ATTEMPTS; ++tries) {
        Cell candidate = {std::rand() % GRID_CELLS, std::rand() % GRID_CELLS};
        if (freeForDrop(candidate) && reachable[candidate.y][candidate.x]) {
            drop = candidate;
            dropVisible = true;
            dropMovesLeft = settings().dropLifetimeMoves;
            return;
        }
    }

    // This also works if the board is almost full.
    for (int y = 0; y < GRID_CELLS; ++y) {
        for (int x = 0; x < GRID_CELLS; ++x) {
            Cell candidate = {x, y};
            if (freeForDrop(candidate) && reachable[candidate.y][candidate.x]) {
                drop = candidate;
                dropVisible = true;
                dropMovesLeft = settings().dropLifetimeMoves;
                return;
            }
        }
    }
}

void randomLayout() {
    // Put one bed in each quadrant so two beds cannot be in planting range
    // at the same time. Their exact positions change on every new run.
    const int left = 2;
    const int right = 12;
    const int top = 2;
    const int bottom = 12;
    for (int attempt = 0; attempt < MAX_LAYOUT_ATTEMPTS; ++attempt) {
        beds[0] = {left + std::rand() % 6, top + std::rand() % 6};
        beds[1] = {right + std::rand() % 6, top + std::rand() % 6};
        beds[2] = {left + std::rand() % 6, bottom + std::rand() % 6};
        beds[3] = {right + std::rand() % 6, bottom + std::rand() % 6};
        rockCount = 0;
        bool completed = true;
        RockWarning noWarning = {false, -1, {0, 0}, 0};
        for (int i = 0; i < settings().startRocks; ++i) {
            bool placed = false;
            int start = std::rand() % (GRID_CELLS * GRID_CELLS);
            for (int cell = 0; cell < GRID_CELLS * GRID_CELLS; ++cell) {
                int choice = (start + cell) % (GRID_CELLS * GRID_CELLS);
                Cell candidate = {choice % GRID_CELLS,
                                  choice / GRID_CELLS};
                if (candidate.x < 1 || candidate.x >= GRID_CELLS - 1 ||
                    candidate.y < 1 || candidate.y >= GRID_CELLS - 1 ||
                    hasRock(candidate) || hasBed(candidate) ||
                    distanceTo(candidate, snake[0]) <= 2) continue;
                rocks[rockCount++] = candidate;
                if (safeRockLayout(noWarning, noWarning)) {
                    placed = true;
                    break;
                }
                --rockCount;
            }
            if (!placed) {
                completed = false;
                break;
            }
        }
        if (completed) {
            return;
        }
    }

    // This should never be needed on a 20 by 20 board, but still leaves a
    // playable layout if a later change makes the random search too strict.
    const Cell fallback[BED_COUNT] = {
        {3, 4}, {16, 4}, {3, 15}, {16, 15}
    };
    for (int i = 0; i < BED_COUNT; ++i) {
        beds[i] = fallback[i];
    }
    rockCount = 0;
    RockWarning noWarning = {false, -1, {0, 0}, 0};
    for (int y = 1; y < GRID_CELLS - 1 && rockCount < settings().startRocks; ++y) {
        for (int x = 1; x < GRID_CELLS - 1 && rockCount < settings().startRocks; ++x) {
            Cell candidate = {x, y};
            if (hasBed(candidate) ||
                distanceTo(candidate, snake[0]) <= 2) continue;
            rocks[rockCount++] = candidate;
            if (!safeRockLayout(noWarning, noWarning)) {
                --rockCount;
            }
        }
    }
}

void resetGame(GameState newState) {
    state = newState;
    snakeLength = 3;
    snake[0] = {4, 10};
    snake[1] = {3, 10};
    snake[2] = {2, 10};
    directionX = 1;
    directionY = 0;
    turnCount = 0;
    water = 0;
    totalDrops = 0;
    flowers = 0;
    moveDelay = settings().stepMs;
    lastClockMs = glutGet(GLUT_ELAPSED_TIME);
    stepClockMs = moveRockClockMs = addRockClockMs = 0;
    waterDecayClockMs = 0;
    movingRock.active = addedRock.active = false;
    growOnNextMove = false;
    for (int i = 0; i < BED_COUNT; ++i) {
        planted[i] = false;
        thorns[i].pending = false;
        thorns[i].active = false;
        thorns[i].movesLeft = 0;
        thorns[i].cell = beds[i];
    }
    dropVisible = false;
    randomLayout();
    statusText = "Collect water drops";
    spawnDrop();
    glutPostRedisplay();
}

void plantNearbyBed() {
    if (state != PLAYING) {
        return;
    }
    if (water < WATER_FOR_FLOWER) {
        statusText = "Need 3 water drops";
        return;
    }

    for (int i = 0; i < BED_COUNT; ++i) {
        if (!canPlantBed(i)) {
            continue;
        }
        planted[i] = true;
        ++flowers;
        water = 0;
        waterDecayClockMs = 0;
        if (flowers == settings().flowersToWin) {
            // The game has stopped, so there is no next step to grow into.
            growOnNextMove = false;
            state = WON;
            statusText = "Garden complete!";
        } else {
            growOnNextMove = true;
            bool marked = markNewThorn(i);
            statusText = marked ? "Thorn marked! Keep moving" :
                                  "No safe thorn spot";
            spawnDrop();
        }
        return;
    }

    for (int i = 0; i < BED_COUNT; ++i) {
        if (!planted[i] &&
            distanceTo(snake[0], beds[i]) <= PLANT_RANGE_CELLS) {
            if (!hasSnake(beds[i]) && !bedInPlannedPath(beds[i]) &&
                !plantingKeepsRoutesOpen(i))
                statusText = "Flower would block a path";
            else
                statusText = "Steer away from the bed";
            return;
        }
    }
    statusText = "Get within 2 tiles of a bed";
}

void changeSpeed(int amount) {
    moveDelay += amount;
    if (moveDelay < MIN_DELAY_MS) {
        moveDelay = MIN_DELAY_MS;
    }
    if (moveDelay > MAX_DELAY_MS) {
        moveDelay = MAX_DELAY_MS;
    }
}

// ========================================
// RENDERING
// ========================================
void drawQuad(float x, float y, float width, float height) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(width, height, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, unitSquare);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();
}

void drawText(float x, float y, const std::string& words,
              void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (std::string::size_type i = 0; i < words.size(); ++i) {
        glutBitmapCharacter(font, static_cast<unsigned char>(words[i]));
    }
}

void drawGrid() {
    glColor3f(0.93f, 0.96f, 0.84f);
    drawQuad(0, 0, BOARD_PIXELS, BOARD_PIXELS);

    glColor3f(0.81f, 0.88f, 0.75f);
    glLineWidth(1);
    glBegin(GL_LINES);
    for (int i = 1; i < GRID_CELLS; ++i) {
        glVertex2i(i * TILE_PIXELS, 0);
        glVertex2i(i * TILE_PIXELS, BOARD_PIXELS);
        glVertex2i(0, i * TILE_PIXELS);
        glVertex2i(BOARD_PIXELS, i * TILE_PIXELS);
    }
    glEnd();
}

void drawBeds() {
    for (int i = 0; i < BED_COUNT; ++i) {
        float x = static_cast<float>(beds[i].x * TILE_PIXELS);
        float y = static_cast<float>(beds[i].y * TILE_PIXELS);
        glColor3f(0.54f, 0.35f, 0.22f);
        drawQuad(x + 2, y + 2, 26, 26);
        glColor3f(0.69f, 0.48f, 0.29f);
        drawQuad(x + 5, y + 5, 20, 20);
        if (planted[i]) {
            glColor3f(0.18f, 0.48f, 0.29f);
            glLineWidth(3);
            glBegin(GL_LINES);
            glVertex2f(x + 15, y + 23);
            glVertex2f(x + 15, y + 12);
            glEnd();
            glColor3f(0.87f, 0.38f, 0.50f);
            drawQuad(x + 11, y + 6, 8, 7);
            drawQuad(x + 7, y + 10, 7, 8);
            drawQuad(x + 16, y + 10, 7, 8);
            glColor3f(1.0f, 0.82f, 0.31f);
            drawQuad(x + 12, y + 11, 6, 6);
        } else {
            glColor3f(0.93f, 0.80f, 0.55f);
            glLineWidth(2);
            glBegin(GL_LINES);
            glVertex2f(x + 10, y + 15);
            glVertex2f(x + 20, y + 15);
            glVertex2f(x + 15, y + 10);
            glVertex2f(x + 15, y + 20);
            glEnd();

            // A bright outline means this bed can be planted right now.
            if (canPlantBed(i)) {
                glColor3f(0.20f, 0.78f, 0.39f);
                glLineWidth(3);
                glBegin(GL_LINE_LOOP);
                glVertex2f(x + 2, y + 2);
                glVertex2f(x + 28, y + 2);
                glVertex2f(x + 28, y + 28);
                glVertex2f(x + 2, y + 28);
                glEnd();
            }
        }
    }
}

void drawRocks() {
    // The original rock stays solid until its move warning finishes.
    for (int i = 0; i < rockCount; ++i) {
        float x = static_cast<float>(rocks[i].x * TILE_PIXELS);
        float y = static_cast<float>(rocks[i].y * TILE_PIXELS);
        glColor3f(0.38f, 0.45f, 0.42f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + 4, y + 24);
        glVertex2f(x + 15, y + 5);
        glVertex2f(x + 27, y + 24);
        glEnd();
        glColor3f(0.55f, 0.62f, 0.56f);
        drawQuad(x + 7, y + 22, 17, 3);
    }
}

void drawRockWarnings() {
    const RockWarning warnings[2] = {movingRock, addedRock};
    for (int i = 0; i < 2; ++i) {
        if (!warnings[i].active) {
            continue;
        }
        float x = static_cast<float>(warnings[i].target.x * TILE_PIXELS);
        float y = static_cast<float>(warnings[i].target.y * TILE_PIXELS);
        if (i == 0) glColor3f(0.77f, 0.27f, 0.79f);
        else glColor3f(0.19f, 0.54f, 0.93f);
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + 3, y + 3);
        glVertex2f(x + 27, y + 3);
        glVertex2f(x + 27, y + 27);
        glVertex2f(x + 3, y + 27);
        glEnd();
        drawText(x + 10, y + 21, i == 0 ? "M" : "A",
                 GLUT_BITMAP_HELVETICA_18);
    }
}

void drawThorns() {
    for (int i = 0; i < BED_COUNT; ++i) {
        if (!thorns[i].pending && !thorns[i].active) {
            continue;
        }
        float x = static_cast<float>(thorns[i].cell.x * TILE_PIXELS);
        float y = static_cast<float>(thorns[i].cell.y * TILE_PIXELS);
        if (thorns[i].pending) {
            glColor3f(0.96f, 0.61f, 0.20f);
            glLineWidth(3);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x + 3, y + 3);
            glVertex2f(x + 27, y + 3);
            glVertex2f(x + 27, y + 27);
            glVertex2f(x + 3, y + 27);
            glEnd();
            char count[5];
            std::snprintf(count, sizeof(count), "%d", thorns[i].movesLeft);
            drawText(x + 11, y + 21, count, GLUT_BITMAP_HELVETICA_18);
        } else {
            glColor3f(0.42f, 0.25f, 0.30f);
            drawQuad(x + 3, y + 21, 24, 6);
            glColor3f(0.78f, 0.23f, 0.32f);
            glBegin(GL_TRIANGLES);
            glVertex2f(x + 3, y + 22);
            glVertex2f(x + 9, y + 5);
            glVertex2f(x + 14, y + 22);
            glVertex2f(x + 11, y + 22);
            glVertex2f(x + 17, y + 5);
            glVertex2f(x + 22, y + 22);
            glVertex2f(x + 18, y + 22);
            glVertex2f(x + 23, y + 9);
            glVertex2f(x + 27, y + 22);
            glEnd();
        }
    }
}

void drawWaterDrop() {
    if (dropVisible) {
        float x = static_cast<float>(drop.x * TILE_PIXELS);
        float y = static_cast<float>(drop.y * TILE_PIXELS);
        glColor3f(0.10f, 0.62f, 0.85f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + 15, y + 4);
        glVertex2f(x + 7, y + 20);
        glVertex2f(x + 23, y + 20);
        glEnd();
        drawQuad(x + 9, y + 18, 12, 6);
        glColor3f(0.78f, 0.96f, 1.0f);
        drawQuad(x + 12, y + 16, 3, 4);
    }
}

void drawSnake() {
    for (int i = snakeLength - 1; i >= 0; --i) {
        float x = static_cast<float>(snake[i].x * TILE_PIXELS);
        float y = static_cast<float>(snake[i].y * TILE_PIXELS);
        if (i == 0) glColor3f(0.15f, 0.45f, 0.29f);
        else glColor3f(0.28f, 0.68f, 0.39f);
        drawQuad(x + 2, y + 2, 26, 26);
        if (i == 0) {
            glColor3f(0.98f, 0.97f, 0.75f);
            drawQuad(x + 8, y + 9, 5, 5);
            drawQuad(x + 18, y + 9, 5, 5);
        }
    }
}

void drawBoardBorder() {
    glColor3f(0.27f, 0.45f, 0.31f);
    drawQuad(0, 0, BOARD_PIXELS, 4);
    drawQuad(0, BOARD_PIXELS - 4, BOARD_PIXELS, 4);
    drawQuad(0, 0, 4, BOARD_PIXELS);
    drawQuad(BOARD_PIXELS - 4, 0, 4, BOARD_PIXELS);
}

// Preserve drawing order so overlapping objects look exactly the same.
void drawGarden() {
    drawGrid();
    drawBeds();
    drawRocks();
    drawRockWarnings();
    drawThorns();
    drawWaterDrop();
    drawSnake();
    drawBoardBorder();
}

void drawPanel() {
    glColor3f(0.09f, 0.23f, 0.20f);
    drawQuad(BOARD_PIXELS, 0, WINDOW_WIDTH_PIXELS - BOARD_PIXELS, WINDOW_HEIGHT_PIXELS);
    glColor3f(0.99f, 0.92f, 0.65f);
    drawText(621, 40, "GARDEN SNAKE");
    glColor3f(0.85f, 0.95f, 0.84f);
    drawText(621, 74, settings().name, GLUT_BITMAP_HELVETICA_12);

    glColor3f(0.17f, 0.44f, 0.38f);
    drawQuad(SLOWER_BUTTON_X, SPEED_BUTTON_Y,
             SPEED_BUTTON_WIDTH, SPEED_BUTTON_HEIGHT);
    drawQuad(FASTER_BUTTON_X, SPEED_BUTTON_Y,
             SPEED_BUTTON_WIDTH, SPEED_BUTTON_HEIGHT);
    glColor3f(0.99f, 0.92f, 0.65f);
    drawText(SLOWER_BUTTON_X + 20, SPEED_BUTTON_Y + 19,
             "SLOWER", GLUT_BITMAP_HELVETICA_12);
    drawText(FASTER_BUTTON_X + 22, SPEED_BUTTON_Y + 19,
             "FASTER", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.85f, 0.95f, 0.84f);

    char label[80];
    std::snprintf(label, sizeof(label), "Flowers: %d / %d", flowers, settings().flowersToWin);
    drawText(621, 141, label);
    std::snprintf(label, sizeof(label), "Rocks: %d / %d", rockCount, settings().maxRocks);
    drawText(621, 167, label, GLUT_BITMAP_HELVETICA_12);
    drawText(621, 187, "WATER METER", GLUT_BITMAP_HELVETICA_12);
    for (int i = 0; i < WATER_FOR_FLOWER; ++i) {
        if (i < water) glColor3f(0.15f, 0.69f, 0.88f);
        else glColor3f(0.35f, 0.46f, 0.43f);
        drawQuad(622 + i * 52, 196, 44, 17);
    }
    glColor3f(0.85f, 0.95f, 0.84f);
    if (dropVisible) {
        std::snprintf(label, sizeof(label), "Drop: %d moves left", dropMovesLeft);
        drawText(621, 232, label, GLUT_BITMAP_HELVETICA_12);
    } else if (water == WATER_FOR_FLOWER) {
        drawText(621, 232, "Water full - press Space",
                 GLUT_BITMAP_HELVETICA_12);
    }
    std::snprintf(label, sizeof(label), "Speed: %d ms / step", moveDelay);
    drawText(621, 252, label, GLUT_BITMAP_HELVETICA_12);
    if (movingRock.active)
        std::snprintf(label, sizeof(label), "Move WARNING: %.1fs",
                     movingRock.remainingMs / 1000.0);
    else
        std::snprintf(label, sizeof(label), "Rock move: %.1fs",
                     (settings().moveRockEveryMs - moveRockClockMs) / 1000.0);
    drawText(621, 273, label, GLUT_BITMAP_HELVETICA_12);

    if (addedRock.active)
        std::snprintf(label, sizeof(label), "Add WARNING: %.1fs",
                     addedRock.remainingMs / 1000.0);
    else if (rockCount >= settings().maxRocks)
        std::snprintf(label, sizeof(label), "Rocks at maximum");
    else
        std::snprintf(label, sizeof(label), "Rock add: %.1fs",
                     (settings().addRockEveryMs - addRockClockMs) / 1000.0);
    drawText(621, 294, label, GLUT_BITMAP_HELVETICA_12);

    if (settings().waterDecayEveryMs == 0)
        std::snprintf(label, sizeof(label), "Held water: never dries");
    else if (water == 0)
        std::snprintf(label, sizeof(label), "Held water: empty");
    else
        std::snprintf(label, sizeof(label), "Held water: %.1fs",
                     (settings().waterDecayEveryMs - waterDecayClockMs) / 1000.0);
    drawText(621, 315, label, GLUT_BITMAP_HELVETICA_12);
    drawText(621, 344, "Purple M / Blue A: rocks",
             GLUT_BITMAP_HELVETICA_12);
    drawText(621, 380, "CONTROLS", GLUT_BITMAP_HELVETICA_12);
    drawText(621, 400, "Arrows: steer", GLUT_BITMAP_HELVETICA_12);
    drawText(621, 420, "Green bed: Space to plant", GLUT_BITMAP_HELVETICA_12);
    drawText(621, 440, "P: pause    R: restart", GLUT_BITMAP_HELVETICA_12);
    drawText(621, 460, "+ / - or click: speed", GLUT_BITMAP_HELVETICA_12);
    drawText(621, 480, "M: difficulty menu", GLUT_BITMAP_HELVETICA_12);
    drawText(621, 490, "Orange: thorn warning", GLUT_BITMAP_HELVETICA_12);
    drawText(621, 510, "Red: thorn obstacle", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.99f, 0.92f, 0.65f);
    drawText(621, 549, "STATUS", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.85f, 0.95f, 0.84f);
    drawText(621, 572, statusText, GLUT_BITMAP_HELVETICA_12);
}

void drawOverlay(const std::string& heading, const std::string& line1,
                 const std::string& line2,
                 const std::string& footer = "Enter or R: new run   1-3: mode") {
    glColor3f(0.09f, 0.23f, 0.20f);
    drawQuad(70, 180, 460, 214);
    glColor3f(0.99f, 0.92f, 0.65f);
    drawText(105, 229, heading);
    glColor3f(0.85f, 0.95f, 0.84f);
    drawText(105, 273, line1, GLUT_BITMAP_HELVETICA_12);
    drawText(105, 298, line2, GLUT_BITMAP_HELVETICA_12);
    drawText(105, 358, footer, GLUT_BITMAP_HELVETICA_12);
}

void drawMenu() {
    glColor3f(0.09f, 0.23f, 0.20f);
    drawQuad(65, 147, 470, 308);
    glColor3f(0.99f, 0.92f, 0.65f);
    drawText(91, 189, "CHOOSE DIFFICULTY");
    glColor3f(0.85f, 0.95f, 0.84f);
    drawText(91, 217, "Press 1, 2 or 3 then Enter to start",
             GLUT_BITMAP_HELVETICA_12);
    for (int i = 0; i < 3; ++i) {
        int y = 230 + i * 40;
        if (i == difficultyIndex) {
            glColor3f(0.17f, 0.44f, 0.38f);
            drawQuad(88, static_cast<float>(y), 422, 34);
        }
        glColor3f(0.99f, 0.92f, 0.65f);
        char label[90];
        std::snprintf(label, sizeof(label), "%d  %-6s  %d ms  %d flowers  %d rocks",
                     i + 1, DIFFICULTIES[i].name, DIFFICULTIES[i].stepMs,
                     DIFFICULTIES[i].flowersToWin,
                     DIFFICULTIES[i].startRocks);
        drawText(99, static_cast<float>(y + 23), label,
                 GLUT_BITMAP_HELVETICA_12);
    }
    glColor3f(0.85f, 0.95f, 0.84f);
    drawText(91, 380, "Arrows steer   Space plants   Mouse changes speed",
             GLUT_BITMAP_HELVETICA_12);
    drawText(91, 414, "R starts a fresh layout on the selected mode",
             GLUT_BITMAP_HELVETICA_12);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    drawGarden();
    drawPanel();
    if (state == MENU) {
        drawMenu();
    } else if (state == PAUSED) {
        drawOverlay("PAUSED", "The snake is waiting.", "Press P to resume.",
                    "R: restart    Esc: quit");
    } else if (state == WON) {
        char message[80];
        std::snprintf(message, sizeof(message), "You grew %d flowers on %s mode!",
                     settings().flowersToWin, settings().name);
        drawOverlay("YOU WIN!", message,
                    "Try again for a new challenge.");
    } else if (state == LOST) {
        drawOverlay("GAME OVER", "The snake hit an obstacle.",
                    "Press Enter or R to try again.");
    }
    glutSwapBuffers();
}

void reshape(int width, int height) {
    windowWidth = width > 0 ? width : 1;
    windowHeight = height > 0 ? height : 1;
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

// ========================================
// MOVEMENT AND GAME CLOCK
// ========================================
void moveSnake() {
    if (state == PLAYING) {
        // Apply one queued turn per step so quick key presses keep their order.
        if (turnCount > 0) {
            directionX = queuedTurns[0].x;
            directionY = queuedTurns[0].y;
            for (int i = 1; i < turnCount; ++i) {
                queuedTurns[i - 1] = queuedTurns[i];
            }
            --turnCount;
        }
        Cell head = {snake[0].x + directionX, snake[0].y + directionY};
        bool willGrow = growOnNextMove && snakeLength < MAX_SNAKE;
        if (head.x < 0 || head.x >= GRID_CELLS || head.y < 0 || head.y >= GRID_CELLS ||
            hasRock(head) || hasFlower(head) || hasThorn(head)) {
            state = LOST;
            statusText = "Collision!";
        } else {
            // The tail moves away unless this move can actually grow it.
            int cellsToCheck = snakeLength - (willGrow ? 0 : 1);
            for (int i = 0; i < cellsToCheck; ++i) {
                if (sameCell(head, snake[i])) {
                    state = LOST;
                    statusText = "You hit your tail!";
                    break;
                }
            }
        }

        if (state == PLAYING) {
            if (willGrow) {
                ++snakeLength;
            }
            growOnNextMove = false;
            for (int i = snakeLength - 1; i > 0; --i) {
                snake[i] = snake[i - 1];
            }
            snake[0] = head;

            if (dropVisible && sameCell(head, drop)) {
                ++water;
                ++totalDrops;
                waterDecayClockMs = 0;
                if (water == WATER_FOR_FLOWER)
                    statusText = "Get near a bed then Space";
                else statusText = "Nice! Find more water";
                spawnDrop();
            } else if (dropVisible) {
                --dropMovesLeft;
                if (dropMovesLeft <= 0) {
                    spawnDrop();
                    statusText = "Water drop dried up";
                }
            }
            advanceThorns();
            // A pending thorn can temporarily occupy the head's square.
            // Try again once a later movement clears any blocked spawn.
            if (water < WATER_FOR_FLOWER && !dropVisible) spawnDrop();
        }
    }
}

void update(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    int elapsedMs = now - lastClockMs;
    lastClockMs = now; // Pausing also refreshes the baseline.
    if (elapsedMs < 0) {
        elapsedMs = 0;
    }

    if (state == PLAYING) {
        // Warnings and water use seconds, independent of snake speed.
        advanceGameClocks(elapsedMs);
        stepClockMs += elapsedMs;
        if (stepClockMs >= moveDelay) {
            stepClockMs %= moveDelay;
            moveSnake();
        }
        glutPostRedisplay();
    }
    glutTimerFunc(CLOCK_TICK_MS, update, 0);
}

// ========================================
// INPUT HANDLING
// ========================================
void keyboard(unsigned char key, int, int) {
    if (key == 27) std::exit(0);
    if ((key == 'm' || key == 'M') && state != MENU) resetGame(MENU);
    else if (key == 'r' || key == 'R') resetGame(PLAYING);
    else if (key == 13 && (state == MENU || state == WON || state == LOST)) {
        resetGame(PLAYING);
    } else if (key >= '1' && key <= '3' &&
               (state == MENU || state == WON || state == LOST)) {
        difficultyIndex = key - '1';
        resetGame(MENU);
    } else if (key == 'p' || key == 'P') {
        if (state == PLAYING) {
            state = PAUSED;
            turnCount = 0;
        }
        else if (state == PAUSED) state = PLAYING;
    } else if (key == ' ') {
        plantNearbyBed();
    } else if ((key == '+' || key == '=') &&
               (state == PLAYING || state == PAUSED)) {
        changeSpeed(-SPEED_STEP_MS);
    } else if ((key == '-' || key == '_') &&
               (state == PLAYING || state == PAUSED)) {
        changeSpeed(SPEED_STEP_MS);
    }
    glutPostRedisplay();
}

void specialKey(int key, int, int) {
    if (state != PLAYING) {
        return;
    }
    int proposedX = 0;
    int proposedY = 0;
    if (key == GLUT_KEY_UP) { proposedX = 0; proposedY = -1; }
    else if (key == GLUT_KEY_DOWN) { proposedX = 0; proposedY = 1; }
    else if (key == GLUT_KEY_LEFT) { proposedX = -1; proposedY = 0; }
    else if (key == GLUT_KEY_RIGHT) { proposedX = 1; proposedY = 0; }
    else return;

    int lastX = turnCount > 0 ? queuedTurns[turnCount - 1].x : directionX;
    int lastY = turnCount > 0 ? queuedTurns[turnCount - 1].y : directionY;
    if ((proposedX == lastX && proposedY == lastY) ||
        (proposedX == -lastX && proposedY == -lastY) ||
        turnCount == TURN_BUFFER_SIZE) return;

    queuedTurns[turnCount++] = {proposedX, proposedY};
    glutPostRedisplay();
}

void mouse(int button, int buttonState, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || buttonState != GLUT_DOWN ||
        (state != PLAYING && state != PAUSED)) return;
    float logicalX = static_cast<float>(x) * WINDOW_WIDTH_PIXELS / windowWidth;
    float logicalY = static_cast<float>(y) * WINDOW_HEIGHT_PIXELS / windowHeight;
    if (logicalY < SPEED_BUTTON_Y ||
        logicalY >= SPEED_BUTTON_Y + SPEED_BUTTON_HEIGHT) return;

    if (logicalX >= SLOWER_BUTTON_X &&
        logicalX < SLOWER_BUTTON_X + SPEED_BUTTON_WIDTH) {
        changeSpeed(SPEED_STEP_MS);
    } else if (logicalX >= FASTER_BUTTON_X &&
               logicalX < FASTER_BUTTON_X + SPEED_BUTTON_WIDTH) {
        changeSpeed(-SPEED_STEP_MS);
    } else {
        return;
    }
    glutPostRedisplay();
}

// ========================================
// APPLICATION ENTRY POINT
// ========================================
int main(int argc, char** argv) {
    std::srand(static_cast<unsigned int>(std::time(0)));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS);
    glutCreateWindow("Garden Snake: Grow to Win");
    glClearColor(0.09f, 0.23f, 0.20f, 1.0f);
    resetGame(MENU);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    glutMouseFunc(mouse);
    glutTimerFunc(CLOCK_TICK_MS, update, 0);
    glutMainLoop();
    return 0;
}
