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

	// 1st rectangle
	float vertices[] = { 0.5f, 0.5f, 0.0f, // top right vertice
	0.5f, -0.5f, 0.0f, // bottom right vertice
	-0.5f, -0.5f, 0.0f, // bottom left verice
	-0.5f, 0.5f, 0.0f, // top left vertice
	}; // This is format organization to tell people that I'm trying to render the rectangle vertices

	// Index starts at 0 in a programming sense but common-sense knowledge would believe that the index starts at 1
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3 // second triangle
	};

	// Generate the buffer ID here
	glGenBuffers(1, &VBO); // The & is a reference to the unsigned int of VBO and converts it to a GLuint pointer type

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &VAO);

	// Generate an element buffer object's ID
	glGenBuffers(1, &EBO);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Bind the vertex array object using its ID
	glBindVertexArray(VAO);

	// Bind the element buffer object inside an element array buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// Place the element buffer object calls between a bind and unbind call, hence why GL_ELEMENT_ARRAY_BUFFER is used
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
	
	// glDrawElements function uses the indices from the EBO that is bounded to the element array buffer
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0); // We don't want to bind the vertex array right now (learned it inside the OpenGL document)
	
	// This function renders the shape in any type of polygon mode
	// The first argument below will apply the front and back triangles and that will help change the polygon mode
	// The second argument is the actual polygon mode: So GL_LINE renders the triangles in a line (wireframe mode)
	// Or we can render the whole triangle like previously by calling in GL_FILL instead of GL_LINE (default mode)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}