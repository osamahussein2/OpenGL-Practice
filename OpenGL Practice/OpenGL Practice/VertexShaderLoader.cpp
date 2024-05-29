#include "VertexShaderLoader.h"
#include "ShaderProgram.h"


VertexShaderLoader::VertexShaderLoader(const char* vertexShaderPath_)
{
	// Make sure the vertex shader file ifstream will throw an exception error
	vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// Open the vertex shader file
		vertexShaderFile.open(vertexShaderPath_);

		// Read the vertex shader's file buffer contents into a stream of the vertex shader file
		vertexShaderStream << vertexShaderFile.rdbuf();

		// Close the vertex shader's file handlers
		vertexShaderFile.close();

		// Convert the vertex shader stream into a vertex shader string
		vertexShaderCode = vertexShaderStream.str();
	}

	catch (std::ifstream::failure error)
	{
		std::cout << "ERROR::SHADER::VERTEX::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}

	vShaderCode = vertexShaderCode.c_str();

	VAO = NULL;
	VBO = NULL;

	EBO = NULL;

	vertexShader = NULL;
}

void VertexShaderLoader::InitializeVertexShaderLoader()
{
	// Create a shader object ID to use it when we create the shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create a vertex shader using OpenGL's version of it

	// Attach the shader source code (the GLSL one) to the shader object and compile it
	glShaderSource(vertexShader, 1, &vShaderCode, NULL);
	glCompileShader(vertexShader);

	int successfullyCompiled; // An integer that checks if the vertex shader compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	// Returns a successful compilation of the vertex shader if it can compile successfully
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successfullyCompiled);

	// If the compilation failed, then return a log compilation error and explain the error
	if (!successfullyCompiled)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << compilationInformationLog << std::endl;
	}
}

void VertexShaderLoader::InitializeVertexObjects()
{
	/* Apparently making the vertices a local variable makes the error go away compared to making it a global
	variable inside the class itself, even though the variable is private to every other class, including Main.cpp */

	// The vertices x, y, and z values must be in range of -1.0 and 1.0 for the object to render on the OpenGL window
	/* If one of the values goes below - 1.0 or above 1.0, the object will not be rendered on the window, meaning
	the object will go offscreen */

	// Initialize the first triangle's vertices and its colors to be at the middle of the OpenGL window
	float vertices[] = { -0.5f, -0.5f, 0.0f,	0.0f, 1.0f, 0.0f,
	0.0f, 0.5f, 0.0f,	0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.0f,	1.0f, 0.0f, 0.0f };

	// Generate the buffer ID here
	glGenBuffers(1, &VBO); // The & is a reference to the unsigned int of VBO and converts it to a GLuint pointer type

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &VAO);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Bind the vertex array object using its ID
	glBindVertexArray(VAO);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set the position attribute's location to 0 like our vertex shader GLSL file
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); // Position attribute location occurs at 1

	// Set the color attribute's location to 1 like our vertex shader GLSL file
	// 3 * sizeof(float) in the last argument below is the offset of the color which is 3 * of our position offset
	// The color offset is 12 bytes, and the position offset is at 0 and goes up to 3 bytes each time
	// Since we have 3 position values, the first color offset will occur at 12 bytes, hence why we multiply it by 3
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1); // Color attribute location occurs at 1

	ShaderProgram shaderProg;

	shaderProg.InitializeShaderProgram();
	glBindVertexArray(VAO);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}