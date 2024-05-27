#include "VertexShaderLoader.h"
#include "ShaderProgram.h"


VertexShaderLoader::VertexShaderLoader()
{
	// I was going to initialize the vertices array but for some reason, I had a lot of trouble initializing it

	// Initialize our vertex shader's source code
	vertexShaderSource = "#version 330 core // Version of GLSL with OpenGL's version\n"
		"layout (location = 0) in vec3 position;\n"
		"void main() \n"
		"{\n"
		"	// gl_Position is a 4D vector inside of OpenGL\n"
		"	gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
		"}\0";

	vertexShader = NULL;

	VAO = NULL;
	VBO = NULL;
}

void VertexShaderLoader::InitializeVertexShaderLoader()
{
	// Create a shader object ID to use it when we create the shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create a vertex shader using OpenGL's version of it

	// Attach the shader source code (the GLSL one) to the shader object and compile it
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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

	float vertices[] = { -0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	0.0f, 0.5f, 0.0f }; // This is format organization to tell people that I'm trying to render the triangle vertices

	// Generate the buffer ID here
	glGenBuffers(1, &VBO); // The & is a reference to the unsigned int of VBO and converts it to a GLuint pointer type

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &VAO);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Bind the vertex array object using its ID
	glBindVertexArray(VAO);

	// Copies the previously defined vertex data into the buffer's memory (probably allocating that memory into the GPU?)
	// The 4th parameter below specifies how we want the graphics card to manage the vertex data
	// GL_STATIC_DRAW only sets the data once and is used many times
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// How OpenGL will interpret the vertex data (per vertex attribute)

	// The first parameter wants to know which vertex attribute we want to modify at a specific location (0 in the GLSL)
	// The second parameter specifies the size of the vertex attribute (we set it to a vec3 in the GLSL file, so it's 3)
	// The third parameter wants to know what type of data of the vertex attribute we want to pass in (it's GL_FLOAT)
	// The fourth parameter wants to know if this type of data should be normalized, we don't need to do that here
	// The fifth parameter passes in the space between consecutive vertex attributes (the next set of position is 3)
	// The final parameter here is an offset of where the vertex's position data begins in the buffer (void* of index 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	ShaderProgram shaderProg;

	shaderProg.InitializeShaderProgram();
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}