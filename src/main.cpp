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
float rotation = M_PI/2 + 0.01; // rotation in radians
float speed = 0.5;
float rotationSpeed = 0.03;
int playerSize = 10;
int numSlices = 128; // Number of rays

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

struct VerticesIndices
{
    std::vector<float> vertices; // For OpenGL rendering
    std::vector<uint> indices;   // For OpenGL rendering
};

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

struct RayInfo{
    float distance; // Distance to the wall hit
    float angle;    // Angle of the ray
    int mapHit; // Map info of the wall hit
    bool hitEW; // True if ray hit east or west wall, False if hit north or south wall
};

struct RayLinesResult {
    std::vector<float> lineVertices; // For OpenGL line drawing
    std::vector<RayInfo> hitInfo;    // Hit info for projection
};

VerticesIndices generateProjectionInfo(std::vector<RayInfo> rayHitInfo)
{
    VerticesIndices projectionInfo;
    projectionInfo.vertices.clear();
    projectionInfo.indices.clear();

    float ivar = (windowWidth - 512.0f) / numSlices;
    float height_scalar = 0.5f;
    int texSize = 16; // Number of vertical slices per wall

    for (int i = 0; i < numSlices; ++i)
    {
        RayInfo ray = rayHitInfo[i];
        float start_x = 512.0f + i * ivar;
        float dist = ray.distance;
        bool sideV = !ray.hitEW; // True if vertical wall, False if horizontal
        int map_type = ray.mapHit;
        float rot = ray.angle;

        float slice_height = 64.0f * windowHeight / dist * height_scalar;
        float start_y = windowHeight / 2.0f - slice_height / 2.0f;
        float y_slice = slice_height / texSize;

        int tx = 0;
        if (sideV) {
            // Vertical wall
            tx = int(fmod(ray.distance, 64.0f) / 4.0f);
            if (rot < M_PI)
                tx = 15 - tx;
        } else {
            // Horizontal wall
            tx = int(fmod(ray.distance, 64.0f) / 4.0f);
            if (rot > M_PI / 2.0f && rot < 3 * M_PI / 2.0f)
                tx = 15 - tx;
        }

        for (int ty = 0; ty < texSize; ++ty)
        {
            float rect_top = start_y + ty * y_slice;
            float rect_bottom = rect_top + y_slice + 1.0f;

            // TODO: Get color for this slice
            std::vector<float> color;
            if (sideV) color = { 1.0f, 1.0f, 1.0f };
            else color = { 0.8f, 0.8f, 0.8f };

            // Rectangle vertices (BL, BR, TL, TR)
            float lX = pixelToScreenX((int)start_x);
            float rX = pixelToScreenX((int)(start_x + ivar));
            float bY = pixelToScreenY((int)rect_bottom);
            float tY = pixelToScreenY((int)rect_top);

            std::vector<float> rectVerts = generateRect(lX, rX, bY, tY, color);
            uint vertOffset = projectionInfo.vertices.size() / 6; // 6 floats per vertex
            projectionInfo.vertices.insert(projectionInfo.vertices.end(), rectVerts.begin(), rectVerts.end());

            uint indices[6] = {
                vertOffset + 0, vertOffset + 1, vertOffset + 2,
                vertOffset + 2, vertOffset + 3, vertOffset + 1
            };
            projectionInfo.indices.insert(projectionInfo.indices.end(), std::begin(indices), std::end(indices));
        }
    }
    return projectionInfo;
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

RayLinesResult generateRayLinesAndDistances() {
    RayLinesResult result;
    result.lineVertices.clear();
    result.hitInfo.clear();

    float fov = 1.7f; // FOV in radians (approx 97 degrees)
    int half_range = numSlices / 2;
    float pz = 0.0f;

    for (int i = -half_range; i < half_range; ++i) {
        RayInfo hitInfo;
        float dtheta = fov * i * M_PI / 180.0f * (64.0f / numSlices);
        float theta = rotation + dtheta;
        hitInfo.angle = theta;
        if (theta > 2 * M_PI) theta -= 2 * M_PI;
        else if (theta < 0) theta += 2 * M_PI;
        float tanth = tan(theta);
        float atanth = 1.0f / tanth;

        // --- Vertical grid intersections ---
        float rx = playerX, ry = playerY;
        float dx, dy;
        if (theta < M_PI / 2.0f || theta > 3.0f * M_PI / 2.0f) {
            rx = std::ceil(playerX / sq) * sq + 0.0001f;
            dx = sq;
        } else {
            rx = std::floor(playerX / sq) * sq - 0.0001f;
            dx = -sq;
        }
        ry = playerY + (rx - playerX) * tanth;
        dy = dx * tanth;
        if (ry > windowHeight) { ry = windowHeight; rx = playerX + (ry - playerY) * atanth; }
        else if (ry < 0) { ry = 0; rx = playerX + (ry - playerY) * atanth; }
        float vrx = rx, vry = ry;
        while (true) {
            int grid_x = int(vrx / sq);
            int grid_y = int(mp - vry / sq);
            if (grid_x < 0 || grid_x >= mp || grid_y < 0 || grid_y >= mp || mapArray[grid_y * mp + grid_x] != 0) {
                hitInfo.mapHit = mapArray[grid_y * mp + grid_x];
                break;
            }
            vrx += dx;
            vry += dy;
        }
        std::pair<float, float> v_rayloc = {vrx, vry};

        // --- Horizontal grid intersections ---
        rx = playerX; ry = playerY;
        if (theta < M_PI) {
            ry = std::ceil(playerY / sq) * sq + 0.0001f;
            dy = sq;
        } else {
            ry = std::floor(playerY / sq) * sq - 0.0001f;
            dy = -sq;
        }
        rx = playerX + (ry - playerY) * atanth;
        dx = dy * atanth;
        if (rx > windowWidth) { rx = windowWidth; ry = playerY + (rx - playerX) * tanth; }
        else if (rx < 0) { rx = 0; ry = playerY + (rx - playerX) * tanth; }
        float hrx = rx, hry = ry;
        while (true) {
            int grid_x = int(hrx / sq);
            int grid_y = int(mp - hry / sq);
            if (grid_x < 0 || grid_x >= mp || grid_y < 0 || grid_y >= mp || mapArray[grid_y * mp + grid_x] != 0) {
                hitInfo.mapHit = mapArray[grid_y * mp + grid_x];
                break;
            }
            hrx += dx;
            hry += dy;
        }
        std::pair<float, float> h_rayloc = {hrx, hry};

        // --- Find shortest ray ---
        float hx = h_rayloc.first, hy = h_rayloc.second;
        float hdx = hx - playerX, hdy = hy - playerY;
        float h_dist = std::sqrt(hdx * hdx + hdy * hdy) * std::cos(dtheta);

        float vx = v_rayloc.first, vy = v_rayloc.second;
        float vdx = vx - playerX, vdy = vy - playerY;
        float v_dist = std::sqrt(vdx * vdx + vdy * vdy) * std::cos(dtheta);

        float rx_final, ry_final;
        if (h_dist < v_dist) {
            rx_final = hx;
            ry_final = hy;
            hitInfo.distance = h_dist;
            hitInfo.hitEW = true;
        } else {
            rx_final = vx;
            ry_final = vy;
            hitInfo.distance = v_dist;
            hitInfo.hitEW = false;
        }

        // Convert to OpenGL screen space
        float glStartX = pixelToScreenX((int)playerX);
        float glStartY = pixelToScreenY((int)playerY);
        float glEndX = pixelToScreenX((int)rx_final);
        float glEndY = pixelToScreenY((int)ry_final);

        // Line: player -> hit
        result.lineVertices.insert(result.lineVertices.end(), {
            glStartX, glStartY, pz, 1.0f, 1.0f, 1.0f,
            glEndX,   glEndY,   pz, 1.0f, 1.0f, 1.0f
        });
        result.hitInfo.insert(result.hitInfo.begin(), hitInfo);
    }
    return result;
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


    // Map vertices and indices
    std::vector<float> mapVertices = generateMapVertices();
    std::vector<uint> mapIndices = generateMapIndices();

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


    // Player vertices and indices
    std::vector<float> playerVertices = generatePlayerVertices();
    std::vector<uint> playerIndices = { // 0 = BL, 1 = BR, 2 = TL, 3 = TR
        0, 1, 2, // Bottom left triangle
        2, 3, 1  // Top right triangle
    };

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


    // Ray lines result which contains vertices and distances
    RayLinesResult rayLinesResult;
    std::vector<float> rayLineVertices; // Format: [x0, y0, z0, r0, g0, b0, x1, y1, z1, r1, g1, b1, ...]
    std::vector<RayInfo> rayHitInfo; // Raycast hit info for projection

    // Create reference containers for rayLines VAO and VBO
    GLuint rayLinesVAO, rayLinesVBO;

    glGenVertexArrays(1, &rayLinesVAO);
    glGenBuffers(1, &rayLinesVBO);
    glBindVertexArray(rayLinesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rayLinesVBO);
    // Initially empty, user will update rayLineVertices and glBufferData as needed
    glBufferData(GL_ARRAY_BUFFER, rayLineVertices.size() * sizeof(float), rayLineVertices.data(), GL_STATIC_DRAW);
    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    // Projection vertices and indices
    std::vector<float> projectionVertices;
    std::vector<float> projectionIndices;

    // Create reference containers for VAO, VBO, and EBO
    GLuint projectionVAO, projectionVBO, projectionEBO;

    // Generate and asign buffers
    glGenVertexArrays(1, &projectionVAO);
    glGenBuffers(1, &projectionVBO);
    glGenBuffers(1, &projectionEBO);

    // Bind VAO so OpenGL knows to use it
    glBindVertexArray(projectionVAO);

    // 1. Bind the VBO specifying that it's a GL_ARRAY_BUFFER
    // 2. Introduce the vertices into the VBO
    glBindBuffer(GL_ARRAY_BUFFER, projectionVBO);
    glBufferData(GL_ARRAY_BUFFER, projectionVertices.size() * sizeof(float), projectionVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, projectionEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, projectionIndices.size() * sizeof(unsigned int), projectionIndices.data(), GL_STATIC_DRAW);

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

        // Bind the map VAO so OpenGL knows to use it
        // Draw the triangle using the GL_TRIANGLES primitive
        glUniform2f(playerPosLocation, 0.0f, 0.0f);
        glBindVertexArray(mapVAO);
        glDrawElements(GL_TRIANGLES, mapIndices.size(), GL_UNSIGNED_INT, 0);

        // Generate rayLineVertices
        rayLinesResult = generateRayLinesAndDistances();
        rayLineVertices = rayLinesResult.lineVertices;
        rayHitInfo = rayLinesResult.hitInfo;

        // Draw rays from player to each endpoint in rayLineVertices
        if (!rayLineVertices.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, rayLinesVBO);
            glBindVertexArray(rayLinesVAO);
            for (size_t i = 0; i < rayLineVertices.size(); i += 6) {
                float endX = rayLineVertices[i];
                float endY = rayLineVertices[i+1];
                float endZ = rayLineVertices[i+2];
                // Color is at i+3, i+4, i+5
                float startX = pixelToScreenX((int)playerX);
                float startY = pixelToScreenY((int)playerY);
                float startZ = 0.0f;
                float colorR = rayLineVertices[i+3];
                float colorG = rayLineVertices[i+4];
                float colorB = rayLineVertices[i+5];
                float lineVerts[12] = {
                    startX, startY, startZ, colorR, colorG, colorB,
                    endX,   endY,   endZ,   colorR, colorG, colorB
                };
                glBufferData(GL_ARRAY_BUFFER, sizeof(lineVerts), lineVerts, GL_DYNAMIC_DRAW);
                glDrawArrays(GL_LINES, 0, 2);
            }
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }


        // Bind the projection VAO so OpenGL knows to use it
        // Generate projection vertices and indices
        VerticesIndices projectionInfo = generateProjectionInfo(rayHitInfo);

        glBindBuffer(GL_ARRAY_BUFFER, projectionVBO);
        glBufferData(GL_ARRAY_BUFFER, projectionInfo.vertices.size() * sizeof(float), projectionInfo.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, projectionEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, projectionInfo.indices.size() * sizeof(unsigned int), projectionInfo.indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(projectionVAO);
        glDrawElements(GL_TRIANGLES, projectionInfo.indices.size(), GL_UNSIGNED_INT, 0);


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

        static double lastTime = glfwGetTime();
        static int nbFrames = 0;
        nbFrames++;
        double currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0) {
            std::cout << "FPS: " << nbFrames << std::endl;
            nbFrames = 0;
            lastTime += 1.0;
        }
    }

    // Delete objects we've created
    glDeleteVertexArrays(1, &rayLinesVAO);
    glDeleteBuffers(1, &rayLinesVBO);
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
