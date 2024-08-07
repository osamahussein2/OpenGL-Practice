#include "ShaderProgram.h"

ShaderProgram::ShaderProgram()
{
	vertexShaderLoader = new VertexShaderLoader("VertexShader.glsl");
	fragmentShaderLoader = new FragmentShaderLoader("FragmentShader.glsl");

	timer = 0.0f;
	moveRight = 0.0f;
	movingPositionLocation = 0;

	shaderProgram = NULL;

	visibilityTextureValue = 0.2f;
}

ShaderProgram::~ShaderProgram()
{
	// Delete the vertex and fragment shaders after the shader objects have been linked with the shader program
	glDeleteShader(vertexShaderLoader->vertexShader);
	glDeleteShader(fragmentShaderLoader->fragmentShader);
}

void ShaderProgram::InitializeShaderProgram()
{
	// Create a shader program that will render both the vertex and fragment shaders to the window
	shaderProgram = glCreateProgram();

	vertexShaderLoader->InitializeVertexShaderLoader();
	fragmentShaderLoader->InitializeFragmentShaderLoader();

	// Attach the vertex and fragment shaders to our created shader program ID object
	glAttachShader(shaderProgram, vertexShaderLoader->vertexShader);
	glAttachShader(shaderProgram, fragmentShaderLoader->fragmentShader);

	// Link the attached vertex and fragment shaders together into one shader program
	glLinkProgram(shaderProgram);

	int successfullyCompiled; // An integer that checks if the shader program compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &successfullyCompiled);

	if (!successfullyCompiled)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << compilationInformationLog << std::endl;
	}

	timer = glfwGetTime(); // Gets the time in seconds using the GLFW library
	moveRight = ((timer) / 5.0f) + 0.1f;

	// Query the location of the uniform color in the fragment shader
	// Pass in the shader program and the uniform color's name inside the glGetUniformLocation defined parameters
	movingPositionLocation = glGetUniformLocation(shaderProgram, "movingPosition");

	// Activate the shader program using glUseProgram function to use it
	glUseProgram(shaderProgram); // This helps set the uniform on the currently running shader program (updating it)

	// Set the uniform value
	glUniform3f(movingPositionLocation, moveRight, 0.0f, 0.0f);

	glUniform1i(glGetUniformLocation(shaderProgram, "theFirstTexture"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "theSecondTexture"), 1);
	glUniform1f(glGetUniformLocation(shaderProgram, "visibilityTexture"), visibilityTextureValue);
}