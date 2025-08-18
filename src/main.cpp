#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Vertex shader source code
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

// Fragment shader source code
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
"}\n\0";

int main()
{
    // Initialize GLFW
    if (!glfwInit())
        return -1;

    // Give GLFW context for which OpenGL versions and profile we are using
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // CORE contains all the modern functions


    // Coordinates of triangle vertices
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f,  // Bottom left
         0.5f, -0.5f, 0.0f,  // Bottom right
         0.0f,  0.5f, 0.0f   // Top center
    };

    // Declare GLFW window with params (width, height, title, fullscreen y/n, and irrelevant)
    GLFWwindow* window = glfwCreateWindow(1024, 512, "Raycast", nullptr, nullptr);
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
    glViewport(0, 0, 1024, 512);


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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 1. Configure the Vertex Attribute so that OpenGL knows how to read the VBO
    // 2. Enable the Vertex Attribute so that OpenGL knows to use it
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);

    // Bind both the VBO and VAO to 0 so we don't accidentally modify them
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // 1. Specify background color
    // 2. Clean back buffer and assign new color
    // 3. Swap back buffer with front buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);

    // Loop for while window is open
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // Tell OpenGL which shader program we want to use
        glUseProgram(shaderProgram);
        // Bind the VAO so OpenGL knows to use it
        glBindVertexArray(VAO);
        // Draw the triangle using the GL_TRIANGLES primitive
        glDrawArrays(GL_TRIANGLES, 0, 3);
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