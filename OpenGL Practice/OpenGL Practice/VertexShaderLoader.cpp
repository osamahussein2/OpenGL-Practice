#include "VertexShaderLoader.h"
#include "ShaderProgram.h"


VertexShaderLoader::VertexShaderLoader()
{
	// I was going to initialize the vertices array but for some reason, I had a lot of trouble initializing it

	// Initialize our vertex shader's source code
	vertexShaderSource[0] = "#version 330 core // Version of GLSL with OpenGL's version\n"
		"layout (location = 0) in vec3 position; // vertex attribute position is equal to 0\n"
		"layout (location = 1) in vec3 color; // color attribute position is equal to 1\n"
		"\n"
		"out vec3 vertexColor; // The sending vertex shader to use inside the fragment shader\n"
		"\n"
		"void main()\n"
		"{\n"
		"	// gl_Position is a 4D vector inside of OpenGL\n"
		"	gl_Position = vec4(position, 1.0);\n"
		"	\n"
		"	// Input the color from the vertex data\n"
		"	vertexColor = color;\n"
		"}\0";

	vertexShaderSource[1] = "#version 330 core // Version of GLSL with OpenGL's version\n"
		"layout (location = 1) in vec3 position;\n"

		"void main()\n"
		"{\n"
		"	// gl_Position is a 4D vector inside of OpenGL\n"
		"	gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
		"}\0";

	vertexShaderSource[2] = "#version 330 core // Version of GLSL with OpenGL's version\n"
		"layout (location = 2) in vec3 position;\n"

		"void main()\n"
		"{\n"
		"	// gl_Position is a 4D vector inside of OpenGL\n"
		"	gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
		"}\0";

	size_t sizeOfVertexShadersArray = sizeof(vertexShader);

	for (int vertexShaders = 0; vertexShaders > sizeOfVertexShadersArray; vertexShaders++)
	{
		vertexShader[vertexShaders] = NULL;
	}

	VAO = NULL;
	VBO = NULL;

	EBO = NULL;
}

void VertexShaderLoader::InitializeVertexShaderLoader()
{
	// Create a shader object ID to use it when we create the shader
	vertexShader[0] = glCreateShader(GL_VERTEX_SHADER); // Create a vertex shader using OpenGL's version of it

	// Attach the shader source code (the GLSL one) to the shader object and compile it
	glShaderSource(vertexShader[0], 1, &vertexShaderSource[0], NULL);
	glCompileShader(vertexShader[0]);

	int successfullyCompiled; // An integer that checks if the vertex shader compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	// Returns a successful compilation of the vertex shader if it can compile successfully
	glGetShaderiv(vertexShader[0], GL_COMPILE_STATUS, &successfullyCompiled);

	// If the compilation failed, then return a log compilation error and explain the error
	if (!successfullyCompiled)
	{
		glGetShaderInfoLog(vertexShader[0], 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER1::VERTEX::COMPILATION_FAILED\n" << compilationInformationLog << std::endl;
	}
}

void VertexShaderLoader::InitializeVertexShaderLoader2()
{
	// Create a shader object ID to use it when we create the shader
	vertexShader[1] = glCreateShader(GL_VERTEX_SHADER); // Create a vertex shader using OpenGL's version of it

	// Attach the shader source code (the GLSL one) to the shader object and compile it
	glShaderSource(vertexShader[1], 1, &vertexShaderSource[1], NULL);
	glCompileShader(vertexShader[1]);

	int successfullyCompiled; // An integer that checks if the vertex shader compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	// Returns a successful compilation of the vertex shader if it can compile successfully
	glGetShaderiv(vertexShader[1], GL_COMPILE_STATUS, &successfullyCompiled);

	// If the compilation failed, then return a log compilation error and explain the error
	if (!successfullyCompiled)
	{
		glGetShaderInfoLog(vertexShader[1], 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER2::VERTEX::COMPILATION_FAILED\n" << compilationInformationLog << std::endl;
	}
}

void VertexShaderLoader::InitializeVertexShaderLoader3()
{
	// Create a shader object ID to use it when we create the shader
	vertexShader[2] = glCreateShader(GL_VERTEX_SHADER); // Create a vertex shader using OpenGL's version of it

	// Attach the shader source code (the GLSL one) to the shader object and compile it
	glShaderSource(vertexShader[2], 1, &vertexShaderSource[2], NULL);
	glCompileShader(vertexShader[2]);

	int successfullyCompiled; // An integer that checks if the vertex shader compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	// Returns a successful compilation of the vertex shader if it can compile successfully
	glGetShaderiv(vertexShader[2], GL_COMPILE_STATUS, &successfullyCompiled);

	// If the compilation failed, then return a log compilation error and explain the error
	if (!successfullyCompiled)
	{
		glGetShaderInfoLog(vertexShader[2], 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER3::VERTEX::COMPILATION_FAILED\n" << compilationInformationLog << std::endl;
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

	// Render triangle 1

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

void VertexShaderLoader::LoadThreeTriangles()
{
	// Initialize the first triangle's vertices to be at the middle of the OpenGL window
	float TriangleOneVertices[] = { -0.5f, -0.5f, 0.0f,
	0.0f, 0.5f, 0.0f,
	0.5f, -0.5f, 0.0f };

	// Initialize the second triangle's vertices to be the right of the OpenGL window
	float TriangleTwoVertices[] = { 1.0f, 0.5f, 0.0f,
	0.0f, 0.5f, 0.0f,
	0.5f, -0.5f, 0.0f };

	// Initialize the third triangle's vertices to be the left of the OpenGL window
	float TriangleThreeVertices[] = { -1.0f, 0.5f, 0.0f,
	0.0f, 0.5f, 0.0f,
	-0.5f, -0.5f, 0.0f };

	// Render triangle 1
	unsigned int VBOtriangleOne, VAOtriangleOne;

	// Generate the buffer ID here
	glGenBuffers(1, &VBOtriangleOne); // The & is a reference to the unsigned int of VBO and converts it to a GLuint pointer type

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &VAOtriangleOne);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, VBOtriangleOne);

	// Bind the vertex array object using its ID
	glBindVertexArray(VAOtriangleOne);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleOneVertices), TriangleOneVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	ShaderProgram shaderProg;

	shaderProg.InitializeShaderProgram();
	glBindVertexArray(VAOtriangleOne);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Render triangle 2
	unsigned int VBOtriangleTwo, VAOtriangleTwo;

	// Generate the buffer ID here
	glGenBuffers(1, &VBOtriangleTwo);

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &VAOtriangleTwo);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, VBOtriangleTwo);

	// Bind the vertex array object using its ID
	glBindVertexArray(VAOtriangleTwo);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleTwoVertices), TriangleTwoVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	shaderProg.InitializeShaderProgram2();

	glBindVertexArray(VAOtriangleTwo);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Render triangle 3
	unsigned int VBOtriangleThree, VAOtriangleThree;

	// Generate the buffer ID here
	glGenBuffers(1, &VBOtriangleThree);

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &VAOtriangleThree);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, VBOtriangleThree);

	// Bind the vertex array object using its ID
	glBindVertexArray(VAOtriangleThree);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleThreeVertices), TriangleThreeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2); // My vertex's location for the third triangle is 2

	shaderProg.InitializeShaderProgram3();

	glBindVertexArray(VAOtriangleThree);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}