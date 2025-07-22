#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#define GLEW_STATIC 1
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;
using namespace std;

// === At top with includes ===
GLuint createTexturedSphereVAO(unsigned int rings, unsigned int sectors, unsigned int &indexCount)
{
    std::vector<vec3> vertices;
    std::vector<vec2> uvs;
    std::vector<unsigned int> indices;

    float const R = 1.0f / float(rings - 1);
    float const S = 1.0f / float(sectors - 1);

    for (unsigned int r = 0; r < rings; ++r)
    {
        for (unsigned int s = 0; s < sectors; ++s)
        {
            float const y = sin(-glm::half_pi<float>() + glm::pi<float>() * r * R);
            float const x = cos(2 * glm::pi<float>() * s * S) * sin(glm::pi<float>() * r * R);
            float const z = sin(2 * glm::pi<float>() * s * S) * sin(glm::pi<float>() * r * R);
            vertices.push_back(vec3(x, y, z));
            uvs.push_back(vec2(s * S, r * R));
        }
    }
    for (unsigned int r = 0; r < rings - 1; ++r)
    {
        for (unsigned int s = 0; s < sectors - 1; ++s)
        {
            indices.push_back(r * sectors + s);
            indices.push_back(r * sectors + (s + 1));
            indices.push_back((r + 1) * sectors + (s + 1));

            indices.push_back(r * sectors + s);
            indices.push_back((r + 1) * sectors + (s + 1));
            indices.push_back((r + 1) * sectors + s);
        }
    }

    GLuint vao, vbo[2], ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(2, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    indexCount = indices.size();
    return vao;
}

GLuint loadTexture(const char *path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

std::string readFile(const char *filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string getTexturedSphereVertexShaderSource()
{
    return readFile("shaders/textured_sphere.vert.glsl");
}

std::string getTexturedSphereFragmentShaderSource()
{
    return readFile("shaders/textured_sphere.frag.glsl");
}

std::string getVertexShaderSource()
{
    return readFile("shaders/shader.vert.glsl");
}

std::string getFragmentShaderSource()
{
    return readFile("shaders/shader.frag.glsl");
}

std::string getSkyboxVertexShaderSource()
{
    return readFile("shaders/skybox_vertex.glsl");
}

std::string getSkyboxFragmentShaderSource()
{
    return readFile("shaders/skybox_fragment.glsl");
}

GLuint compileTexturedSphereShader()
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    std::string vsSourceStr = getTexturedSphereVertexShaderSource();
    const char *vsSource = vsSourceStr.c_str();
    glShaderSource(vs, 1, &vsSource, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fsSourceStr = getTexturedSphereFragmentShaderSource();
    const char *fsSource = fsSourceStr.c_str();
    glShaderSource(fs, 1, &fsSource, nullptr);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int compileVertexAndFragShaders()
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderSourceStr = getVertexShaderSource();
    const char *vertexShaderSource = vertexShaderSourceStr.c_str();
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
    const char *fragmentShaderSource = fragmentShaderSourceStr.c_str();
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

int createVertexBufferObject()
{
    // cube model
    vec3 vertexArray[] = {
        // position,                            color
        vec3(-0.5f, -0.5f, -0.5f), vec3(1.0f, 0.0f, 0.0f), //left - red
        vec3(-0.5f, -0.5f, 0.5f),  vec3(1.0f, 0.0f, 0.0f), vec3(-0.5f, 0.5f, 0.5f),  vec3(1.0f, 0.0f, 0.0f),

        vec3(-0.5f, -0.5f, -0.5f), vec3(1.0f, 0.0f, 0.0f), vec3(-0.5f, 0.5f, 0.5f),  vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f, -0.5f),  vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f, 0.5f, -0.5f),   vec3(0.0f, 0.0f, 1.0f), // far - blue
        vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, 0.0f, 1.0f), vec3(-0.5f, 0.5f, -0.5f), vec3(0.0f, 0.0f, 1.0f),

        vec3(0.5f, 0.5f, -0.5f),   vec3(0.0f, 0.0f, 1.0f), vec3(0.5f, -0.5f, -0.5f), vec3(0.0f, 0.0f, 1.0f),
        vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, 0.0f, 1.0f),

        vec3(0.5f, -0.5f, 0.5f),   vec3(0.0f, 1.0f, 1.0f), // bottom - turquoise
        vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, 1.0f, 1.0f), vec3(0.5f, -0.5f, -0.5f), vec3(0.0f, 1.0f, 1.0f),

        vec3(0.5f, -0.5f, 0.5f),   vec3(0.0f, 1.0f, 1.0f), vec3(-0.5f, -0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f),
        vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, 1.0f, 1.0f),

        vec3(-0.5f, 0.5f, 0.5f),   vec3(0.0f, 1.0f, 0.0f), // near - green
        vec3(-0.5f, -0.5f, 0.5f),  vec3(0.0f, 1.0f, 0.0f), vec3(0.5f, -0.5f, 0.5f),  vec3(0.0f, 1.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f),    vec3(0.0f, 1.0f, 0.0f), vec3(-0.5f, 0.5f, 0.5f),  vec3(0.0f, 1.0f, 0.0f),
        vec3(0.5f, -0.5f, 0.5f),   vec3(0.0f, 1.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f),    vec3(1.0f, 0.0f, 1.0f), // right - purple
        vec3(0.5f, -0.5f, -0.5f),  vec3(1.0f, 0.0f, 1.0f), vec3(0.5f, 0.5f, -0.5f),  vec3(1.0f, 0.0f, 1.0f),

        vec3(0.5f, -0.5f, -0.5f),  vec3(1.0f, 0.0f, 1.0f), vec3(0.5f, 0.5f, 0.5f),   vec3(1.0f, 0.0f, 1.0f),
        vec3(0.5f, -0.5f, 0.5f),   vec3(1.0f, 0.0f, 1.0f),

        vec3(0.5f, 0.5f, 0.5f),    vec3(1.0f, 1.0f, 0.0f), // top - yellow
        vec3(0.5f, 0.5f, -0.5f),   vec3(1.0f, 1.0f, 0.0f), vec3(-0.5f, 0.5f, -0.5f), vec3(1.0f, 1.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f),    vec3(1.0f, 1.0f, 0.0f), vec3(-0.5f, 0.5f, -0.5f), vec3(1.0f, 1.0f, 0.0f),
        vec3(-0.5f, 0.5f, 0.5f),   vec3(1.0f, 1.0f, 0.0f)};


    // create a vertex array
    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);


    // upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(
		0, 
		3, 
		GL_FLOAT, 
		GL_FALSE, \
		2 * sizeof(vec3), 
		(void *)0);
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(1,
		3, 
		GL_FLOAT, 
		GL_FALSE, 
		2 * sizeof(vec3), 
		(void *)sizeof(vec3)
    );
    glEnableVertexAttribArray(1);


    return vertexBufferObject;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{ //SKY!
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
				0, 
				GL_RGB, 
				width, 
				height, 
				0, 
				GL_RGB, 
				GL_UNSIGNED_BYTE, 
				data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int compileSkyboxShaderProgram()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderStr = getSkyboxVertexShaderSource();
    const char* vertexShaderSource = vertexShaderStr.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    // TODO: check compile errors

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderStr = getSkyboxFragmentShaderSource();
    const char* fragmentShaderSource = fragmentShaderStr.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    // TODO: check compile errors

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    // TODO: check linking errors

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint createSphereVAO(unsigned int rings, unsigned int sectors, unsigned int &indexCount)
{
    std::vector<vec3> vertices;
    std::vector<vec3> colors;
    std::vector<unsigned int> indices;

    float const R = 1.0f / float(rings - 1);
    float const S = 1.0f / float(sectors - 1);

    for (unsigned int r = 0; r < rings; ++r)
    {
        for (unsigned int s = 0; s < sectors; ++s)
        {
            float const y = sin(-glm::half_pi<float>() + glm::pi<float>() * r * R);
            float const x = cos(2 * glm::pi<float>() * s * S) * sin(glm::pi<float>() * r * R);
            float const z = sin(2 * glm::pi<float>() * s * S) * sin(glm::pi<float>() * r * R);
            vertices.push_back(vec3(x, y, z));
            colors.push_back(vec3(1.0f, 0.0f, 1.0f)); // pink color!
        }
    }

    for (unsigned int r = 0; r < rings - 1; ++r)
    {
        for (unsigned int s = 0; s < sectors - 1; ++s)
        {
            indices.push_back(r * sectors + s);
            indices.push_back(r * sectors + (s + 1));
            indices.push_back((r + 1) * sectors + (s + 1));

            indices.push_back(r * sectors + s);
            indices.push_back((r + 1) * sectors + (s + 1));
            indices.push_back((r + 1) * sectors + s);
        }
    }

    GLuint vao, vbo[2], ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(2, vbo);

    // position buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0); // aPos
    glEnableVertexAttribArray(0);

    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec3), &colors[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0); // aColor
    glEnableVertexAttribArray(1);

    // indices
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    indexCount = indices.size();
    return vao;
}

int main(int argc, char *argv[])
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// glfw window
    GLFWwindow *window = glfwCreateWindow(800, 600, "Comp371 - Lab 03", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // disable the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // init glew
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // compile base shaders
    int shaderProgram = compileVertexAndFragShaders();

    glUseProgram(shaderProgram);

    // compile skybox shader
    unsigned int skyboxShaderProgram = compileSkyboxShaderProgram();
    glUseProgram(skyboxShaderProgram);
    glUniform1i(glGetUniformLocation(skyboxShaderProgram, "skybox"), 0); 
	// set sampler to texture unit 0

    // load skybox cubemap textures
    std::vector<std::string> faces = {
		"textures/skybox1/1.png",
        "textures/skybox1/2.png",
        "textures/skybox1/3.png",
        "textures/skybox1/4.png",
        "textures/skybox1/5.png",
        "textures/skybox1/6.png"
	};
    unsigned int cubemapTexture = loadCubemap(faces);

    // camera parameters for view transform
    vec3 cameraPosition(0.6f, 1.0f, 10.0f);
    vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
    vec3 cameraUp(0.0f, 1.0f, 0.0f);

    // other camera parameters
    float cameraSpeed = 3.0f;
    float cameraFastSpeed = 2 * cameraSpeed;
    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;
    bool cameraFirstPerson = true; // press 1 or 2 to toggle this variable

    // spinning cube at camera position
    float spinningCubeAngle = 0.0f;

    // define floating orb 1 parameters
    float orbAngle = 0.0f; // degrees
    float orbRadius = 2.0f;
    float orbHeight = 1.0f;
    float orbSize = 0.2f; // slightly bigger than your spinning cube

    // Set projection matrix for shader, this won't change
    mat4 projectionMatrix = glm::perspective(
		70.0f,
        800.0f / 600.0f,
        0.01f,
        100.0f
	);

    GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Set initial view matrix
    mat4 viewMatrix = lookAt(
		cameraPosition, 
		cameraPosition + cameraLookAt, 
		cameraUp
		);

    GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);


    // define and upload geometry to the GPU
    int vao = createVertexBufferObject();

    unsigned int moonIndexCount = 0;
    GLuint moonVAO = createTexturedSphereVAO(40, 40, moonIndexCount);
    GLuint moonTexture = loadTexture("textures/moon.jpg");

    unsigned int earthIndexCount = 0;
    GLuint earthVAO = createTexturedSphereVAO(40, 40, earthIndexCount);
    GLuint earthTexture = loadTexture("textures/earth.jpg");

    GLuint orbShader = compileTexturedSphereShader();

    GLuint sunTexture = loadTexture("textures/sun.jpg");
    unsigned int sunIndexCount = 0;
    GLuint sunVAO = createTexturedSphereVAO(40, 40, sunIndexCount);


    // for frame time
    float lastFrameTime = glfwGetTime();
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);

    // pause state
    bool isPaused = false;
    bool wasSpacePressed = false;

    // enable Backface culling
    glEnable(GL_CULL_FACE);

    // enable depth testing
    glEnable(GL_DEPTH_TEST);


    float skyboxVertices[] = {-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                              -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                              1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                              -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};


    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // frame time calculation
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;

        // Handle spacebar toggle for pause
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (!wasSpacePressed) {
                isPaused = !isPaused;
                wasSpacePressed = true;
            }
        } else {
            wasSpacePressed = false;
        }

        // Store the original dt for camera and cube movement
        float animationDt = isPaused ? 0.0f : dt;

        // clear depth and color buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // first and third person camera toggle
        if (cameraFirstPerson)
        {
            viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
        }
        else
        {
            float radius = 1.5f;
            glm::vec3 position = cameraPosition - radius * cameraLookAt;
            viewMatrix = lookAt(position, position + cameraLookAt, cameraUp);
        }
        GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

        // === RENDER SKYBOX FIRST ===

        // change depth function so skybox passes depth test
        glDepthFunc(GL_LEQUAL);

        // Use your skybox shader
        glUseProgram(skyboxShaderProgram);

        // remove translation for skybox
        mat4 skyboxView = mat4(mat3(viewMatrix));

        glUniformMatrix4fv(
			glGetUniformLocation(
			skyboxShaderProgram, 
			"view"
		), 
			1, 
			GL_FALSE, 
			&skyboxView[0][0]
		);

        glUniformMatrix4fv(
			glGetUniformLocation(
			skyboxShaderProgram, 
			"projection"
		), 
			1, 
			GL_FALSE,
			&projectionMatrix[0][0]
		);

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS); // restore default depth function

        // === REST OF SCENE ===
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

        // draw geometry
        glBindVertexArray(vao);

        // draw ground
        GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");

        spinningCubeAngle += 180.0f * dt;

        // put the cube somewhere in world space
        vec3 cubePosition(0.0f, 1.0f, -5.0f);

        // only render the cube in third-person
        if (!cameraFirstPerson)
        {
            mat4 spinningCubeWorldMatrix = translate(
				mat4(1.0f), cameraPosition
			) * rotate(
					mat4(1.0f), 
					radians(spinningCubeAngle), 
					vec3(0.0f, 1.0f, 0.0f)
				) * scale(
					mat4(1.0f), 
					vec3(0.1f, 0.1f, 0.1f)
					);

            glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &spinningCubeWorldMatrix[0][0]);
            glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // update orb 1 logic
        // update orb angle for animation
        orbAngle += 20.0f * animationDt; // slow circular movement

        // Define fixed sun position as the center of orbit
        vec3 sunPosition = vec3(0.0f, 0.0f, -20.0f);  // Moved back further and centered
        float earthOrbitRadius = 5.0f;  // Larger orbit radius for Earth around the Sun
        
        // compute Earth position (orbiting around the Sun)
        float orbX = sunPosition.x + earthOrbitRadius * cos(radians(orbAngle));
        float orbZ = sunPosition.z + earthOrbitRadius * sin(radians(orbAngle));
        float orbY = sunPosition.y;  // Keep Earth at Sun's height

        mat4 orbWorldMatrix =
            translate(
                mat4(1.0f), 
                vec3(orbX, orbY, orbZ)
            ) * rotate(
                mat4(1.0f),
                radians(orbAngle),  // Rotate Earth on its axis
                vec3(0.0f, 1.0f, 0.0f)
            ) * scale(
                mat4(1.0f), 
                vec3(0.3f)  // Make Earth a bit larger
            );

        // === RENDER SUN ===
        // Update sun rotation
        static float sunRotationAngle = 0.0f;
        sunRotationAngle += 15.0f * animationDt; // Rotate 15 degrees per second

        mat4 sunWorldMatrix = translate(
			mat4(1.0f), 
            sunPosition
        ) * rotate(
            mat4(1.0f),
            radians(sunRotationAngle),
            vec3(0.0f, 1.0f, 0.0f)  // Rotate around Y axis
        ) * scale(mat4(1.0f), vec3(2.0f));  // Made sun bigger

        vec3 lightPos = sunPosition; // same as sun position

        glUseProgram(orbShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        glUniform1i(glGetUniformLocation(orbShader, "texture1"), 0);
        glUniformMatrix4fv(glGetUniformLocation(orbShader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(orbShader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(orbShader, "worldMatrix"), 1, GL_FALSE, &sunWorldMatrix[0][0]);
        glUniform3fv(glGetUniformLocation(orbShader, "lightColor"), 1, &vec3(1.0f, 1.0f, 1.0f)[0]);
        glUniform3fv(glGetUniformLocation(orbShader, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(orbShader, "viewPos"), 1, &cameraPosition[0]);

        glBindVertexArray(sunVAO);
        glDrawElements(GL_TRIANGLES, sunIndexCount, GL_UNSIGNED_INT, 0);


        // === RENDER EARTH (or moon) ===
        glUseProgram(orbShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTexture);
        glUniform1i(glGetUniformLocation(orbShader, "texture1"), 0);

        // set matrices
        glUniformMatrix4fv(glGetUniformLocation(orbShader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(orbShader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(orbShader, "worldMatrix"), 1, GL_FALSE, &orbWorldMatrix[0][0]);

        glUniform3fv(glGetUniformLocation(orbShader, "lightColor"), 1, &vec3(1.0f)[0]);
        glUniform3fv(glGetUniformLocation(orbShader, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(orbShader, "viewPos"), 1, &cameraPosition[0]);


        glBindVertexArray(earthVAO);
        glDrawElements(GL_TRIANGLES, earthIndexCount, GL_UNSIGNED_INT, 0);

        // === Render the Moon orbiting around the Earth ===

        // compute moon position relative to the Earth
        // Moon's orbit speed is relative to Earth's orbit, so we use the same time scale
        float moonOrbitAngle = orbAngle * 4.0f; // moon orbits faster
        float moonOrbitRadius = 1.0f;  // increased radius for visibility

        // Earth's current position is orbX, orbY, orbZ
        float moonX = orbX + moonOrbitRadius * cos(radians(moonOrbitAngle));
        float moonZ = orbZ + moonOrbitRadius * sin(radians(moonOrbitAngle));
        float moonY = orbY; // Keep the moon at the same height as Earth

        mat4 moonWorldMatrix = translate(
			mat4(1.0f), vec3(moonX, moonY, moonZ)
		) * scale(
			mat4(1.0f), 
			vec3(0.08f, 0.08f, 0.08f)
		); // smaller than earth

        glUseProgram(orbShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTexture);
        glUniform1i(glGetUniformLocation(orbShader, "texture1"), 0);

        // set matrices
        glUniformMatrix4fv(glGetUniformLocation(orbShader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(orbShader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(orbShader, "worldMatrix"), 1, GL_FALSE, &moonWorldMatrix[0][0]);

        glBindVertexArray(moonVAO);
        glDrawElements(GL_TRIANGLES, moonIndexCount, GL_UNSIGNED_INT, 0);


        // end Frame
        glfwSwapBuffers(window);
        glfwPollEvents();

        // handle inputs
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
            glfwSetWindowShouldClose(window, true);
		}

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) // move camera down
        {
            cameraFirstPerson = true;
        }

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) // move camera down
        {
            cameraFirstPerson = false;
        }


        // This was solution for Lab02 - Moving camera exercise
        // We'll change this to be a first or third person camera
        bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                       glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;


        // move your mouse left/right to rotate the camera horizontally (yaw)
        // move your mouse up/down to rotate the camera vertically (pitch)

        //get mouse position each frame
        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);

        // calculate movement delta. This gives how much the mouse moved this frame.
        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;

        // store mouse position for next frame
        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;

        // COVERT TO horizontal /vertical angles as mouse moves
        const float cameraAngularSpeed = 10.0f;
        cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
        cameraVerticalAngle -= dy * cameraAngularSpeed * dt;

        // clamp vertical angle SO THAT camera does not move upside down.
        cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));

        // wrap horizontal angle FOR 360 EFFECT
        if (cameraHorizontalAngle > 360)
            cameraHorizontalAngle -= 360;
        else if (cameraHorizontalAngle < -360)
            cameraHorizontalAngle += 360;

        // convert angles to direction vector. now cameraLookAt is the direction the camera should look at
        float theta = radians(cameraHorizontalAngle);
        float phi = radians(cameraVerticalAngle);
        cameraLookAt = vec3(cos(phi) * cos(theta), sin(phi), -cos(phi) * sin(theta));

        // calculate side vector
        vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
        glm::normalize(cameraSideVector);

        // use camera lookat and side vectors to update positions with ASDW
        // adjust code below
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPosition += cameraLookAt * dt * currentCameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPosition -= cameraLookAt * dt * currentCameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPosition += cameraSideVector * dt * currentCameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPosition -= cameraSideVector * dt * currentCameraSpeed;
        }

        const float arrowLookSpeed = 60.0f; // degrees per second

        // arrow keys camera
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            cameraHorizontalAngle += arrowLookSpeed * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            cameraHorizontalAngle -= arrowLookSpeed * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            cameraVerticalAngle += arrowLookSpeed * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            cameraVerticalAngle -= arrowLookSpeed * dt;
        }
    }

    // shutdown GLFW
    glfwTerminate();

    return 0;
}
