#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Vertex shader source code
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 vertexColor;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0);\n"
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
const int sq = 64;
const int mp = 8;

float pixelToScreenX(int x) // x pixel value to screen value
{
    return 2.0f * static_cast<float>(x) / windowWidth - 1.0f;
}

float pixelToScreenY(int y) // y pixel value to screen value
{
    return 2.0f * static_cast<float>(y) / windowHeight - 1.0f;
}

std::vector<float> generateSquare(float lX, float rX, float bY, float tY, std::vector<float> color)
{
    std::vector<float> mapVertices;
    // Bottom left triangle

    // Bottom left vertex
    mapVertices.push_back(lX);
    mapVertices.push_back(bY);
    mapVertices.push_back(0.0f);

    mapVertices.insert(mapVertices.end(), color.begin(), color.end());

    // Bottom right vertex
    mapVertices.push_back(rX);
    mapVertices.push_back(bY);
    mapVertices.push_back(0.0f);

    mapVertices.insert(mapVertices.end(), color.begin(), color.end());

    // Top left vertex
    mapVertices.push_back(lX);
    mapVertices.push_back(tY);
    mapVertices.push_back(0.0f);

    mapVertices.insert(mapVertices.end(), color.begin(), color.end());

    // Top right triangle

    // Top left vertex
    mapVertices.push_back(lX);
    mapVertices.push_back(tY);
    mapVertices.push_back(0.0f);

    mapVertices.insert(mapVertices.end(), color.begin(), color.end());

    // Top right vertex
    mapVertices.push_back(rX);
    mapVertices.push_back(tY);
    mapVertices.push_back(0.0f);

    mapVertices.insert(mapVertices.end(), color.begin(), color.end());

    // Bottom right vertex
    mapVertices.push_back(rX);
    mapVertices.push_back(bY);
    mapVertices.push_back(0.0f);

    mapVertices.insert(mapVertices.end(), color.begin(), color.end());

    return mapVertices;
}

std::vector<float> generateMapVertices(const int* mapArray)
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
                color.push_back(0.0f);
                color.push_back(0.0f);
            } else if (mapAt == 3) {
                color.push_back(0.0f);
                color.push_back(0.0f);
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

            std::vector<float> squareVertices = generateSquare(lX, rX, bY, tY, color);
            mapVertices.insert(mapVertices.end(), squareVertices.begin(), squareVertices.end());
        }
    }

    return mapVertices;
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


    // Map coordinates
    const int mapArray[] = {
        1,1,1,1,1,1,1,1,
        1,0,0,2,0,0,0,1,
        1,0,2,2,0,0,0,1,
        1,0,0,0,0,0,0,1,
        1,0,0,0,0,3,0,1,
        1,0,0,0,0,3,0,1,
        1,0,0,0,0,0,0,1,
        1,1,1,1,1,1,1,1
    };

    std::vector<float> mapVertices = generateMapVertices(mapArray);

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

    // Set viewport to the entire window, params ((botL), (topR))
    glViewport(0, 0, windowWidth, windowHeight);


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
    GLuint VAO, VBO;

    // Generate the VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Make the VAO the current Vertex Array Object by binding it
    glBindVertexArray(VAO);

    // 1. Bind the VBO specifying that it's a GL_ARRAY_BUFFER
    // 2. Introduce the vertices into the VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mapVertices.size() * sizeof(float), mapVertices.data(), GL_STATIC_DRAW);

    // 1. Configure the Vertex Attribute so that OpenGL knows how to read the VBO
    // 2. Enable the Vertex Attribute so that OpenGL knows to use it

    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Bind both the VBO and VAO to 0 so we don't accidentally modify them
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


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

        // Tell OpenGL which shader program we want to use
        glUseProgram(shaderProgram);
        // Bind the VAO so OpenGL knows to use it
        glBindVertexArray(VAO);
        // Draw the triangle using the GL_TRIANGLES primitive
        glDrawArrays(GL_TRIANGLES, 0, mapVertices.size() / 3);
        glfwSwapBuffers(window);

        // Process window events
        glfwPollEvents();
    }

    // Delete objects we've created
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // Terminate and destroy GLFW before the function ends
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
