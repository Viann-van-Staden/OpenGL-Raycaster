#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

// Window dimensions
const int screenWidth = 800;
const int screenHeight = 600;

// Mini-map dimensions and position
const int miniMapSize = 200;
const int miniMapPosX = screenWidth - miniMapSize - 10;
const int miniMapPosY = screenHeight - miniMapSize - 10;

// Map dimensions and definition
const int MAP_WIDTH = 10;
const int MAP_HEIGHT = 10;
int map[MAP_WIDTH][MAP_HEIGHT] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 0, 0, 1},
    {1, 1, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Color structure
struct Color {
    float r, g, b;
};

// Camera structure
struct Camera {
    float x, y;        // Camera position
    float dirX, dirY;  // Camera direction vector
    float planeX, planeY;  // 2D raycaster version of camera plane
    float moveSpeed;   // Movement speed
    float rotSpeed;    // Rotation speed
    float mouseSensitivity; // Mouse sensitivity
    float bobbingAmplitude; // Amplitude of bobbing
    float bobbingFrequency; // Frequency of bobbing
    float bobbingTime; // Time accumulator for bobbing

    Camera(float startX, float startY, float startDirX, float startDirY, float startPlaneX, float startPlaneY, float moveSpeed, float rotSpeed, float mouseSensitivity, float bobbingAmplitude, float bobbingFrequency)
        : x(startX), y(startY), dirX(startDirX), dirY(startDirY), planeX(startPlaneX), planeY(startPlaneY), moveSpeed(moveSpeed), rotSpeed(rotSpeed), mouseSensitivity(mouseSensitivity), bobbingAmplitude(bobbingAmplitude), bobbingFrequency(bobbingFrequency), bobbingTime(0.0f) {}
};

// Initialize camera with movement, rotation speeds, mouse sensitivity, and bobbing parameters
Camera camera(5.0f, 5.0f, -1.0f, 0.01f, 0.1f, 0.66f, 0.01f, 0.01f, 0.001f, 0.1f, 2.0f);

// Array to track key states
bool keys[1024] = { false };

// GLFW error callback function
void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

// Check if the given position collides with a wall
bool isWall(float x, float y) {
    int mapX = static_cast<int>(x);
    int mapY = static_cast<int>(y);

    if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
        return false; // Out of bounds
    }

    return map[mapX][mapY] > 0; // Check for wall
}

// GLFW key callback function
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        keys[key] = true;
    }
    else if (action == GLFW_RELEASE) {
        keys[key] = false;
    }
}

// GLFW mouse callback function
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static double lastX = screenWidth / 2.0;
    static double lastY = screenHeight / 2.0;

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    xoffset *= camera.mouseSensitivity;
    yoffset *= camera.mouseSensitivity;

    // Update camera direction based on mouse input
    float oldDirX = camera.dirX;
    camera.dirX = camera.dirX * cos(-xoffset) - camera.dirY * sin(-xoffset);
    camera.dirY = oldDirX * sin(-xoffset) + camera.dirY * cos(-xoffset);

    float oldPlaneX = camera.planeX;
    camera.planeX = camera.planeX * cos(-xoffset) - camera.planeY * sin(-xoffset);
    camera.planeY = oldPlaneX * sin(-xoffset) + camera.planeY * cos(-xoffset);
}

// Function to initialize OpenGL and GLFW
void initOpenGL(GLFWwindow*& window) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set error callback
    glfwSetErrorCallback(error_callback);

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(screenWidth, screenHeight, "Raycaster", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set viewport
    glViewport(0, 0, screenWidth, screenHeight);

    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1); // Replacing gluOrtho2D with glOrtho
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set key and mouse callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Hide the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

// Function to draw the mini-map
void drawMiniMap() {
    glViewport(miniMapPosX, miniMapPosY, miniMapSize, miniMapSize);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, MAP_WIDTH, 0, MAP_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw the map
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            if (map[x][y] > 0) {
                glColor3f(0.5f, 0.5f, 0.5f); // Wall color
                glBegin(GL_QUADS);
                glVertex2f(x, y);
                glVertex2f(x + 1, y);
                glVertex2f(x + 1, y + 1);
                glVertex2f(x, y + 1);
                glEnd();
            }
        }
    }

    // Draw the player position on the mini-map
    glColor3f(0.0f, 1.0f, 0.0f); // Player color
    float playerSize = 0.2f;
    glBegin(GL_QUADS);
    glVertex2f(camera.x - playerSize, camera.y - playerSize);
    glVertex2f(camera.x + playerSize, camera.y - playerSize);
    glVertex2f(camera.x + playerSize, camera.y + playerSize);
    glVertex2f(camera.x - playerSize, camera.y + playerSize);
    glEnd();

    // Reset the viewport
    glViewport(0, 0, screenWidth, screenHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1); // Reset to main viewport
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Function to render the scene
void renderScene(GLFWwindow* window) {
    // Clear the screen with a background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the top color (above the raycasted scene)
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.7f, 1.0f); // Light blue color for the top
    glVertex2f(0, screenHeight);
    glVertex2f(screenWidth, screenHeight);
    glVertex2f(screenWidth, screenHeight / 2);
    glVertex2f(0, screenHeight / 2);
    glEnd();

    // Draw the bottom color (below the raycasted scene)
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.5f, 0.5f); // Gray color for the bottom
    glVertex2f(0, screenHeight / 2);
    glVertex2f(screenWidth, screenHeight / 2);
    glVertex2f(screenWidth, 0);
    glVertex2f(0, 0);
    glEnd();

    // Update camera bobbing
    float bobbingOffset = camera.bobbingAmplitude * sin(camera.bobbingFrequency * camera.bobbingTime);
    float bobbingOffsetY = bobbingOffset;

    // Handle movement based on key states
    float moveStep = camera.moveSpeed * (keys[GLFW_KEY_LEFT_SHIFT] ? 2.0f : 1.0f); // Double speed when SHIFT is pressed
    if (keys[GLFW_KEY_W]) {
        if (!isWall(camera.x + camera.dirX * moveStep, camera.y)) {
            camera.x += camera.dirX * moveStep;
        }
        if (!isWall(camera.x, camera.y + camera.dirY * moveStep)) {
            camera.y += camera.dirY * moveStep;
        }
    }
    if (keys[GLFW_KEY_S]) {
        if (!isWall(camera.x - camera.dirX * moveStep, camera.y)) {
            camera.x -= camera.dirX * moveStep;
        }
        if (!isWall(camera.x, camera.y - camera.dirY * moveStep)) {
            camera.y -= camera.dirY * moveStep;
        }
    }
    if (keys[GLFW_KEY_A]) {
        if (!isWall(camera.x - camera.planeX * moveStep, camera.y)) {
            camera.x -= camera.planeX * moveStep;
        }
        if (!isWall(camera.x, camera.y - camera.planeY * moveStep)) {
            camera.y -= camera.planeY * moveStep;
        }
    }
    if (keys[GLFW_KEY_D]) {
        if (!isWall(camera.x + camera.planeX * moveStep, camera.y)) {
            camera.x += camera.planeX * moveStep;
        }
        if (!isWall(camera.x, camera.y + camera.planeY * moveStep)) {
            camera.y += camera.planeY * moveStep;
        }
    }

    // Update bobbing time
    camera.bobbingTime += camera.moveSpeed * (keys[GLFW_KEY_LEFT_SHIFT] ? 2.0f : 1.0f) * 0.01f;

    // Raycasting loop for each vertical stripe of pixels on the screen
    for (int x = 0; x < screenWidth; x++) {
        // Calculate ray position and direction
        float cameraX = 2 * x / (float)screenWidth - 1; // X-coordinate in camera space
        float rayDirX = camera.dirX + camera.planeX * cameraX;
        float rayDirY = camera.dirY + camera.planeY * cameraX;

        // Map coordinates the ray is currently in
        int mapX = int(camera.x);
        int mapY = int(camera.y);

        // Length of ray from one x or y-side to next x or y-side
        float sideDistX, sideDistY;

        // Length of ray from one side to next in x and y
        float deltaDistX = (rayDirX == 0) ? 1e30f : std::abs(1.0f / rayDirX);
        float deltaDistY = (rayDirY == 0) ? 1e30f : std::abs(1.0f / rayDirY);
        float perpWallDist;

        // Step and initial sideDist
        int stepX, stepY;

        // Which direction to step in x or y direction (either +1 or -1)
        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (camera.x - mapX) * deltaDistX;
        }
        else {
            stepX = 1;
            sideDistX = (mapX + 1.0f - camera.x) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (camera.y - mapY) * deltaDistY;
        }
        else {
            stepY = 1;
            sideDistY = (mapY + 1.0f - camera.y) * deltaDistY;
        }

        // Perform DDA (Digital Differential Analysis)
        bool hit = false;
        int side;
        while (!hit) {
            // Jump to next map square, either in x-direction, or in y-direction
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            // Check if ray has hit a wall
            if (map[mapX][mapY] > 0) {
                hit = true;
            }
        }

        // Calculate distance of the wall from the camera
        if (side == 0) {
            perpWallDist = (mapX - camera.x + (1 - stepX) / 2) / rayDirX;
        }
        else {
            perpWallDist = (mapY - camera.y + (1 - stepY) / 2) / rayDirY;
        }

        // Calculate height of the line to draw on the screen
        int lineHeight = int(screenHeight / perpWallDist);

        // Calculate the draw start and end points of the line
        int drawStart = -lineHeight / 2 + screenHeight / 2;
        int drawEnd = lineHeight / 2 + screenHeight / 2;

        // Ensure the line is within the screen bounds
        if (drawStart < 0) drawStart = 0;
        if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;

        // Set color based on side
        Color color;
        if (side == 0) {
            color.r = 0.5f;
            color.g = 0.5f;
            color.b = 1.0f;
        }
        else {
            color.r = 1.0f;
            color.g = 0.5f;
            color.b = 0.5f;
        }

        // Draw the vertical line
        glBegin(GL_LINES);
        glColor3f(color.r, color.g, color.b);
        glVertex2f(x, drawStart + bobbingOffsetY);
        glVertex2f(x, drawEnd + bobbingOffsetY);
        glEnd();
    }

    // Draw the mini-map
    drawMiniMap();

    // Swap buffers
    glfwSwapBuffers(window);
}

int main() {
    GLFWwindow* window;
    initOpenGL(window);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        renderScene(window);
        glfwPollEvents();

        // Close the window if ESC is pressed
        if (keys[GLFW_KEY_ESCAPE]) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

    // Clean up
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
