#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

// Vertex shader source code
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 vertexColor;\n"
"uniform vec2 playerPos;\n"
"void main()\n"
"{\n"
"   vec3 worldPos = aPos + vec3(playerPos, 0.0);\n"
"   gl_Position = vec4(worldPos, 1.0);\n"
"   vertexColor = aColor;\n"
"}\0";

// Fragment shader source code
const char* fragmentShaderSource = "#version 330 core\n"
"in vec3 vertexColor;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(vertexColor, 1.0f);\n"
"}\n\0";

const int windowWidth = 1024;
const int windowHeight = 512;
const int sq = 64; // width and height of each square in the grid
const int mp = 8; // how many columns and rows are in the square

// Map coordinates
int mapArray[] = {
    1,1,1,1,1,1,1,1,
    1,0,0,2,0,0,0,1,
    1,0,2,2,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,0,0,3,0,1,
    1,0,0,0,0,3,0,1,
    1,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1
};

float playerX = 256;
float playerY = 256;
float rotation = M_PI/2; // rotation in radians
float speed = 0.5;
float rotationSpeed = 0.05;
int playerSize = 10;

const float sqrhf = sqrt(1.0f/2.0f);

float pixelToScreenX(int x) // x pixel value to screen value
{
    return 2.0f * static_cast<float>(x) / windowWidth - 1.0f;
}

float pixelToScreenY(int y) // y pixel value to screen value
{
    return 2.0f * static_cast<float>(y) / windowHeight - 1.0f;
}

// Returns vertices and colors for a rectangle in order BLV,COL,BRV,COL,TLV,COL,TRV,COL
std::vector<float> generateRect(float lX, float rX, float bY, float tY, std::vector<float> color)
{
    std::vector<float> mapVertices;

    // Bottom left vertex
    mapVertices.push_back(lX);
    mapVertices.push_back(bY);
    mapVertices.push_back(0.0f);

    // Color
    mapVertices.insert(mapVertices.end(), color.begin(), color.end());

    // Bottom right vertex
    mapVertices.push_back(rX);
    mapVertices.push_back(bY);
    mapVertices.push_back(0.0f);

    // Color
    mapVertices.insert(mapVertices.end(), color.begin(), color.end());

    // Top left vertex
    mapVertices.push_back(lX);
    mapVertices.push_back(tY);
    mapVertices.push_back(0.0f);

    // Color
    mapVertices.insert(mapVertices.end(), color.begin(), color.end());

    // Top right vertex
    mapVertices.push_back(rX);
    mapVertices.push_back(tY);
    mapVertices.push_back(0.0f);

    // Color
    mapVertices.insert(mapVertices.end(), color.begin(), color.end());

    return mapVertices;
}

std::vector<float> generateMapVertices()
{
    std::vector<float> mapVertices;

    for (int i = 0; i < mp; i++) {
        for (int j = 0; j < mp; j++) {
            int mapAt = mapArray[(mp - 1 - i) * mp + j];

            std::vector<float> color;
            if (mapAt == 1) {
                color.push_back(1.0f);
                color.push_back(1.0f);
                color.push_back(1.0f);
            } else if (mapAt == 2) {
                color.push_back(1.0f);
                color.push_back(0.4f);
                color.push_back(0.4f);
            } else if (mapAt == 3) {
                color.push_back(0.4f);
                color.push_back(0.4f);
                color.push_back(1.0f);
            } else {
                color.push_back(0.0f);
                color.push_back(0.0f);
                color.push_back(0.0f);
            }

            int offset = 1;
            // Draw wall square
            float lX = pixelToScreenX(j * sq + offset);
            float rX = pixelToScreenX((j + 1) * sq - offset);
            float bY = pixelToScreenY(i * sq + offset);
            float tY = pixelToScreenY((i + 1) * sq - offset);

            std::vector<float> squareVertices = generateRect(lX, rX, bY, tY, color);
            mapVertices.insert(mapVertices.end(), squareVertices.begin(), squareVertices.end());
        }
    }

    return mapVertices;
}

std::vector<uint> generateMapIndices() {
    std::vector<uint> mapIndices;

    for (uint i = 0; i < mp*mp; i++)
    {
        uint offset = i*4;
        uint indices[6] = { // 0 = BL, 1 = BR, 2 = TL, 3 = TR
            // Bottom left triangle
            offset + 0, // Bottom left vertex & color
            offset + 1, // Bottom right vertex & color
            offset + 2, // Top left vertex & color
            // Top right triangle
            offset + 2, // Top left vertex & color
            offset + 3, // Top right vertex & color
            offset + 1  // Bottom right vertex & color
        };
        mapIndices.insert(mapIndices.end(), std::begin(indices), std::end(indices));
    }

    return mapIndices;
}

std::vector<float> generatePlayerVertices()
{
    float halfWidth = (float)playerSize / windowWidth;
    float halfHeight = (float)playerSize / windowHeight;

    std::vector<float> color = {1.0f, 0.5f, 0.5f};

    float lX = -halfWidth, rX = halfWidth;
    float bY = -halfHeight, tY = halfHeight;

    return generateRect(lX, rX, bY, tY, color);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void movePlayer(float signfb, float signlr) {
    if (signfb != 0.0f || signlr != 0.0f) {
        float dx = speed * cos(rotation) * signfb;
        float dy = speed * sin(rotation) * signfb;
        int grid_x = int((playerX + 8 * dx) / 64); // collision detection
        int grid_y = int(mp - (playerY + 8 * dy) / 64);
        if (mapArray[grid_y * mp + grid_x] != 0) {
            return;
        }
        playerX += dx;
        playerY += dy;

        dx = speed * cos(rotation + M_PI/2) * signlr;
        dy = speed * sin(rotation + M_PI/2) * signlr;
        grid_x = int((playerX + 8 * dx) / 64); // collision detection
        grid_y = int(mp - (playerY + 8 * dy) / 64);
        if (mapArray[grid_y * mp + grid_x] != 0) {
            return;
        }
        playerX += dx;
        playerY += dy;
    }
}

void turnPlayer(float dir) {
    rotation += rotationSpeed * dir;
    if (rotation < 0) {
        rotation += 2 * M_PI;
    } else if (rotation > 2 * M_PI) {
        rotation -= 2 * M_PI;
    }
}

int main()
{
    // Initialize GLFW
    if (!glfwInit())
        return -1;

    // Give GLFW context for which OpenGL versions and profile we are using
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // CORE contains all the modern functions

    std::vector<float> mapVertices = generateMapVertices();
    std::vector<uint> mapIndices = generateMapIndices();

    std::vector<float> playerVertices = generatePlayerVertices();
    std::vector<uint> playerIndices = { // 0 = BL, 1 = BR, 2 = TL, 3 = TR
        0, // Bottom left vertex & color
        1, // Bottom right vertex & color
        2, // Top left vertex & color
        2, // Top left vertex & color
        3, // Top right vertex & color
        1  // Bottom right vertex & color
    };

    // Declare GLFW window with params (width, height, title, fullscreen y/n, and irrelevant)
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Raycast", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    // Update context of window to be current
    glfwMakeContextCurrent(window);

    // Load GLAD
    gladLoadGL();

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    // 1. Create vertex shader object and get reference
    // 2. Attach vertex shader source to the vertex shader object
    // 3. Compile the vertex shader into machine code
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // 1. Create fragment shader object and get its reference
    // 2. Attach fragment shader source to the fragment shader object
    // 3. Compile the vertex shader into machine code
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // 1. Create shader program object and get its reference
    // 2. Attach the vertex and fragment shaders to the shader program
    // 3. Wrap up / link all the shaders together into the shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Delete the now useless shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // WHOLE GOAL of this part was to create the shaderProgram:
    // Had to create vertex and frag shaders, attach them, then delete them once no longer needed


    // Create reference containers for the Vertex Array Object and the Vertex Buffer Object
    GLuint mapVAO, mapVBO, mapEBO;

    // Generate the map VAO, VBO, EBO
    glGenVertexArrays(1, &mapVAO);
    glGenBuffers(1, &mapVBO);
    glGenBuffers(1, &mapEBO);

    // Make the VAO the current Vertex Array Object by binding it
    glBindVertexArray(mapVAO);

    // 1. Bind the VBO specifying that it's a GL_ARRAY_BUFFER
    // 2. Introduce the vertices into the VBO
    glBindBuffer(GL_ARRAY_BUFFER, mapVBO);
    glBufferData(GL_ARRAY_BUFFER, mapVertices.size() * sizeof(float), mapVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mapEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mapIndices.size() * sizeof(unsigned int), mapIndices.data(), GL_STATIC_DRAW);

    // 1. Configure the Vertex Attribute so that OpenGL knows how to read the VBO
    // 2. Enable the Vertex Attribute so that OpenGL knows to use it

    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // Create reference containers for the Vertex Array Object and the Vertex Buffer Object
    GLuint playerVAO, playerVBO, playerEBO;

    // Generate the map VAO, VBO, EBO
    glGenVertexArrays(1, &playerVAO);
    glGenBuffers(1, &playerVBO);
    glGenBuffers(1, &playerEBO);

    // Make the VAO the current Vertex Array Object by binding it
    glBindVertexArray(playerVAO);

    // 1. Bind the VBO specifying that it's a GL_ARRAY_BUFFER
    // 2. Introduce the vertices into the VBO
    glBindBuffer(GL_ARRAY_BUFFER, playerVBO);
    glBufferData(GL_ARRAY_BUFFER, playerVertices.size() * sizeof(float), playerVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, playerEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, playerIndices.size() * sizeof(unsigned int), playerIndices.data(), GL_STATIC_DRAW);

    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);



    // Bind both the VBO, VAO, and EBO to 0 so we don't accidentally modify them
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    // 1. Specify background color
    // 2. Clean back buffer and assign new color
    // 3. Swap back buffer with front buffer
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);

    // Loop for while window is open
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float signfb = 0;
        float signlr = 0;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            signfb += 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            signlr += 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            signfb -= 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            signlr -= 1.0f;
        }
        if (signfb != 0 && signlr != 0) {
            signfb *= sqrhf;
            signlr *= sqrhf;
        }
        movePlayer(signfb, signlr);

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            turnPlayer(1);
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            turnPlayer(-1);
        }

        // Tell OpenGL which shader program we want to use
        glUseProgram(shaderProgram);
        GLint playerPosLocation = glGetUniformLocation(shaderProgram, "playerPos");

        // Bind the VAO so OpenGL knows to use it
        // Draw the triangle using the GL_TRIANGLES primitive
        glUniform2f(playerPosLocation, 0.0f, 0.0f);
        glBindVertexArray(mapVAO);
        glDrawElements(GL_TRIANGLES, mapIndices.size(), GL_UNSIGNED_INT, 0);

        // Bind the VAO so OpenGL knows to use it
        // Draw the triangle using the GL_TRIANGLES primitive
        float offX = pixelToScreenX((int)playerX);
        float offY = pixelToScreenY((int)playerY);
        glUniform2f(playerPosLocation, offX, offY);
        glBindVertexArray(playerVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);

        // Process window events
        glfwPollEvents();
    }

    // Delete objects we've created
    glDeleteVertexArrays(1, &mapVAO);
    glDeleteBuffers(1, &mapVBO);
    glDeleteBuffers(1, &mapEBO);
    glDeleteVertexArrays(1, &playerVAO);
    glDeleteBuffers(1, &playerVBO);
    glDeleteBuffers(1, &playerEBO);
    glDeleteProgram(shaderProgram);

    // Terminate and destroy GLFW before the function ends
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
