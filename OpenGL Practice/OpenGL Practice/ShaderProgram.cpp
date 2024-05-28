#include "ShaderProgram.h"

ShaderProgram::ShaderProgram()
{
	size_t sizeOfShaderPrograms = sizeof(shaderProgram);

	for (unsigned int shaderPrograms = 0; shaderPrograms > sizeOfShaderPrograms; shaderPrograms++)
	{
		shaderProgram[shaderPrograms] = NULL;
	}
}

ShaderProgram::~ShaderProgram()
{
	// Delete the vertex and fragment shaders after the shader objects have been linked with the shader program
	glDeleteShader(vertexShaderLoader.vertexShader[0]);
	glDeleteShader(fragmentShaderLoader.fragmentShader[0]);

	glDeleteShader(vertexShaderLoader.vertexShader[1]);
	glDeleteShader(fragmentShaderLoader.fragmentShader[1]);

	glDeleteShader(vertexShaderLoader.vertexShader[2]);
	glDeleteShader(fragmentShaderLoader.fragmentShader[2]);
}

void ShaderProgram::InitializeShaderProgram()
{
	// Shader Program 1
	// Create a shader program that will render both the vertex and fragment shaders to the window
	shaderProgram[0] = glCreateProgram();

	vertexShaderLoader.InitializeVertexShaderLoader();
	fragmentShaderLoader.InitializeFragmentShaderLoader();

	// Attach the vertex and fragment shaders to our created shader program ID object
	glAttachShader(shaderProgram[0], vertexShaderLoader.vertexShader[0]);
	glAttachShader(shaderProgram[0], fragmentShaderLoader.fragmentShader[0]);

	// Link the attached vertex and fragment shaders together into one shader program
	glLinkProgram(shaderProgram[0]);

	int successfullyCompiled; // An integer that checks if the shader program compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	glGetProgramiv(shaderProgram[0], GL_LINK_STATUS, &successfullyCompiled);

	if (!successfullyCompiled)
	{
		glGetProgramInfoLog(shaderProgram[0], 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER1::PROGRAM::LINKING_FAILED\n" << compilationInformationLog << std::endl;
	}

	// Activate the shader program using glUseProgram function to use it
	glUseProgram(shaderProgram[0]);
}

void ShaderProgram::InitializeShaderProgram2()
{
	// Create a shader program that will render both the vertex and fragment shaders to the window
	shaderProgram[1] = glCreateProgram();

	vertexShaderLoader.InitializeVertexShaderLoader2();
	fragmentShaderLoader.InitializeFragmentShaderLoader2();

	// Attach the vertex and fragment shaders to our created shader program ID object
	glAttachShader(shaderProgram[1], vertexShaderLoader.vertexShader[1]);
	glAttachShader(shaderProgram[1], fragmentShaderLoader.fragmentShader[1]);

	// Link the attached vertex and fragment shaders together into one shader program
	glLinkProgram(shaderProgram[1]);

	int successfullyCompiled; // An integer that checks if the shader program compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	glGetProgramiv(shaderProgram[1], GL_LINK_STATUS, &successfullyCompiled);

	if (!successfullyCompiled)
	{
		glGetProgramInfoLog(shaderProgram[1], 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER2::PROGRAM::LINKING_FAILED\n" << compilationInformationLog << std::endl;
	}

	// Activate the shader program using glUseProgram function to use it
	glUseProgram(shaderProgram[1]);
}

void ShaderProgram::InitializeShaderProgram3()
{
	// Create a shader program that will render both the vertex and fragment shaders to the window
	shaderProgram[2] = glCreateProgram();

	vertexShaderLoader.InitializeVertexShaderLoader3();
	fragmentShaderLoader.InitializeFragmentShaderLoader3();

	// Attach the vertex and fragment shaders to our created shader program ID object
	glAttachShader(shaderProgram[2], vertexShaderLoader.vertexShader[2]);
	glAttachShader(shaderProgram[2], fragmentShaderLoader.fragmentShader[2]);

	// Link the attached vertex and fragment shaders together into one shader program
	glLinkProgram(shaderProgram[2]);

	int successfullyCompiled; // An integer that checks if the shader program compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	glGetProgramiv(shaderProgram[1], GL_LINK_STATUS, &successfullyCompiled);

	if (!successfullyCompiled)
	{
		glGetProgramInfoLog(shaderProgram[2], 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER3::PROGRAM::LINKING_FAILED\n" << compilationInformationLog << std::endl;
	}

	// Activate the shader program using glUseProgram function to use it
	glUseProgram(shaderProgram[2]);
}