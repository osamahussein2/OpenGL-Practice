#include "ShaderProgram.h"

ShaderProgram::ShaderProgram()
{
	shaderProgram = NULL;
}

ShaderProgram::~ShaderProgram()
{
	// Delete the vertex and fragment shaders after the shader objects have been linked with the shader program
	glDeleteShader(vertexShaderLoader.vertexShader);
	glDeleteShader(fragmentShaderLoader.fragmentShader);
}

void ShaderProgram::InitializeShaderProgram()
{
	// Create a shader program that will render both the vertex and fragment shaders to the window
	shaderProgram = glCreateProgram();

	vertexShaderLoader.InitializeVertexShaderLoader();
	fragmentShaderLoader.InitializeFragmentShaderLoader();

	// Attach the vertex and fragment shaders to our created shader program ID object
	glAttachShader(shaderProgram, vertexShaderLoader.vertexShader);
	glAttachShader(shaderProgram, fragmentShaderLoader.fragmentShader);

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

	// Activate the shader program using glUseProgram function to use it
	glUseProgram(shaderProgram);
}