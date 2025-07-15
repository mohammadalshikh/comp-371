#include <iostream>

#include <GL/glew.h> // old load

#include <GLFW/glfw3.h> // window and input

#include <glm/glm.hpp> // math and 3d

#include <fstream>

#include <sstream>

#define GLEW_STATIC 1

std::string readFile(const char* filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string getVertexShaderSource()
{
    return readFile("shader.vert.glsl");
}

std::string getFragmentShaderSource()
{
    return readFile("shader.frag.glsl");
}

int compileAndLinkShaders()
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderSourceStr = getVertexShaderSource();
    const char* vertexShaderSource = vertexShaderSourceStr.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderSourceStr = getFragmentShaderSource();
    const char* fragmentShaderSource = fragmentShaderSourceStr.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int createVertexArrayObject()
{
    // A vertex is a point on a polygon (x,y axis), it contains positions and other data
    // (colors, etc.  ) x y z | r g b

    // if red, green, blue, will generate rainbow, else all white
    glm::vec3 vertexArray[] = {
        glm::vec3(0.0f, 0.5f, 0.0f),   // top center position
        glm::vec3(1.0f, 0.0f, 0.0f),   // top center color (red)
        glm::vec3(0.5f, -0.5f, 0.0f),  // bottom right
        glm::vec3(0.0f, 1.0f, 0.0f),   // bottom right color (green)
        glm::vec3(-0.5f, -0.5f, 0.0f), // bottom left
        glm::vec3(0.0f, 0.0f, 1.0f),   // bottom left color (blue)
    };

    // Create a vertex array
    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject); // 1 VAO and its ID
    glBindVertexArray(vertexArrayObject);     // activate it

    // Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

    // aPos , x y z (size) , floats (type)
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(glm::vec3),
        (void *)0
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(glm::vec3),
        (void *)sizeof(glm::vec3)
    );

    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);             // Unbind to not modify the VAO

    return vertexArrayObject;
}

int main(int argc, char *argv[])
{
    // Initialize GLFW and OpenGL version
    glewExperimental = true;
    glfwInit();

    // On windows, we set OpenGL version to 3, to support more hardware
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow *window = glfwCreateWindow(800, 600, "Comp371 - Lab 01", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);


    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Burgundy background
    glClearColor(0.415f, 0.039f, 0.125f, 1.0f);

    // Compile and link shaders here ...
    int shaderProgram = compileAndLinkShaders();

    // Define and upload geometry to the GPU here ...
    int vao = createVertexArrayObject();

    // Entering Main Loop
    while (!glfwWindowShouldClose(window))
    {
        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT);


        glUseProgram(shaderProgram);      // Use the shader program we created
        glBindVertexArray(vao);           // Bind the VAO to use it
        glDrawArrays(GL_TRIANGLES, 0, 3); // Draw the triangle using 3 vertices
        // go from vertex 0 to vertex 3, using the GL_TRIANGLES mode


        // End frame
        glfwSwapBuffers(window);

        // Detect inputs
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    // Shutdown GLFW
    glfwTerminate();

    return 0;
}
