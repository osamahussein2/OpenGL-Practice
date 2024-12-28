#include "GeometryShader.h"
#include "Camera.h"
#include "ShaderProgram.h"
#include "Model.h"

ShaderProgram* geometryShaderProgram = new ShaderProgram();
ShaderProgram* textureShaderProgram = new ShaderProgram();
Model* model = nullptr;

/* Between the vertex and the fragment shader there is an optional shader stage called the geometry shader. A geometry 
shader takes as input a set of vertices that form a single primitive (e.g. a point or a triangle). The geometry shader can 
then transform these vertices as it sees fit before sending them to the next shader stage */

GeometryShader::GeometryShader(const char* geometryShaderPath_)
{
    // Make sure the geometry shader file ifstream will throw an exception error
    geometryShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        if (geometryShaderPath_ != nullptr)
        {
            // Open the geometry shader file
            geometryShaderFile.open(geometryShaderPath_);

            // Read the geometry shader's file buffer contents into a stream of the geometry shader file
            geometryShaderStream << geometryShaderFile.rdbuf();

            // Close the geometry shader's file handlers
            geometryShaderFile.close();

            // Convert the geometry shader stream into a geometry shader string
            geometryShaderCode = geometryShaderStream.str();
        }
    }

    catch (std::ifstream::failure error)
    {
        std::cout << "ERROR::SHADER::GEOMETRY::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }

    if (geometryShaderPath_ != nullptr)
    {
        gShaderCode = geometryShaderCode.c_str();
    }

    pointsVAO = NULL;
    pointsVBO = NULL;

    model = nullptr;
}

GeometryShader::~GeometryShader()
{
    glDeleteVertexArrays(1, &pointsVAO);
    glDeleteBuffers(1, &pointsVBO);

    model = nullptr;
}

void GeometryShader::InitializeGeometryShaderLoader()
{
    // Create a shader object ID to use it when we create the shader
    geometryShader = glCreateShader(GL_GEOMETRY_SHADER); // Create a geometry shader using OpenGL's version of it

    // Attach the shader source code (the GLSL one) to the shader object and compile it
    glShaderSource(geometryShader, 1, &gShaderCode, NULL);
    glCompileShader(geometryShader);

    int successfullyCompiled; // An integer that checks if the geometry shader compilation was successful
    char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

    // Returns a successful compilation of the geometry shader if it can compile successfully
    glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &successfullyCompiled);

    // If the compilation failed, then return a log compilation error and explain the error
    if (!successfullyCompiled)
    {
        glGetShaderInfoLog(geometryShader, 512, NULL, compilationInformationLog);
        std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << compilationInformationLog << std::endl;
    }
}

void GeometryShader::InitializeGeometryModel()
{
    // Geometry Shader part 2 & 3
    textureShaderProgram->InitializeShaderProgram(new VertexShaderLoader("ModelVertexShader.glsl"), new
        FragmentShaderLoader("ModelFragmentShader.glsl"));

    geometryShaderProgram->InitializeShaderProgram(new VertexShaderLoader("GeometryVertexShader.glsl"), new
        FragmentShaderLoader("GeometryFragmentShader.glsl"), new GeometryShader("GeometryShader.glsl"));

    //model = new Model("Models/Nanosuit/Nanosuit.obj"); // Geometry Shader part 2

    stbi_set_flip_vertically_on_load(true);
    model = new Model("Models/Backpack/Backpack.obj"); // Geometry Shader part 3
}

void GeometryShader::InitializeGeometryVertices()
{
    float points[] =
    {
        -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
    };

    glGenBuffers(1, &pointsVBO);
    glGenVertexArrays(1, &pointsVAO);

    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

}

void GeometryShader::DrawGeometryPoints()
{
    glBindVertexArray(pointsVAO);
    glDrawArrays(GL_POINTS, 0, 4);
}

void GeometryShader::DrawExplodingModel()
{
    // Geometry Shader part 2
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), float(800 / 600), 1.0f, 100.0f);
    glm::mat4 viewMatrix = Camera::CameraLookAt();
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    glUseProgram(geometryShaderProgram->shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(geometryShaderProgram->shaderProgram, "projectionMatrix"), 1,
        GL_FALSE, glm::value_ptr(projectionMatrix));

    glUniformMatrix4fv(glGetUniformLocation(geometryShaderProgram->shaderProgram, "viewMatrix"), 1,
        GL_FALSE, glm::value_ptr(viewMatrix));

    glUniformMatrix4fv(glGetUniformLocation(geometryShaderProgram->shaderProgram, "modelMatrix"), 1,
        GL_FALSE, glm::value_ptr(modelMatrix));

    glUniform1f(glGetUniformLocation(geometryShaderProgram->shaderProgram, "time"), glfwGetTime());

    model->DrawModel(geometryShaderProgram);
}

void GeometryShader::DrawVisualizingNormalVectors()
{
    // Geometry Shader part 3

    /* First, draw the scene as normal without a geometry shader and then we draw the scene a second time, but this time 
    only displaying normal vectors that we generate via a geometry shader. The geometry shader takes as input a triangle 
    primitive and generates 3 lines from them in the directions of their normal - one normal vector for each vertex */
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), float(800 / 600), 1.0f, 100.0f);
    glm::mat4 viewMatrix = Camera::CameraLookAt();
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    glUseProgram(textureShaderProgram->shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(textureShaderProgram->shaderProgram, "projectionMatrix"), 1,
        GL_FALSE, glm::value_ptr(projectionMatrix));

    glUniformMatrix4fv(glGetUniformLocation(textureShaderProgram->shaderProgram, "viewMatrix"), 1,
        GL_FALSE, glm::value_ptr(viewMatrix));

    glUniformMatrix4fv(glGetUniformLocation(textureShaderProgram->shaderProgram, "modelMatrix"), 1,
        GL_FALSE, glm::value_ptr(modelMatrix));

    model->DrawModel(textureShaderProgram);

    glUseProgram(geometryShaderProgram->shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(geometryShaderProgram->shaderProgram, "projectionMatrix"), 1,
        GL_FALSE, glm::value_ptr(projectionMatrix));

    glUniformMatrix4fv(glGetUniformLocation(geometryShaderProgram->shaderProgram, "viewMatrix"), 1,
        GL_FALSE, glm::value_ptr(viewMatrix));

    glUniformMatrix4fv(glGetUniformLocation(geometryShaderProgram->shaderProgram, "modelMatrix"), 1,
        GL_FALSE, glm::value_ptr(modelMatrix));

    model->DrawModel(geometryShaderProgram);
}
