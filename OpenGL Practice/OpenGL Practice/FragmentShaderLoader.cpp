#include "FragmentShaderLoader.h"

FragmentShaderLoader::FragmentShaderLoader()
{
	fragmentShaderSource = "#version 330 core\n"
		"out vec4 fragColor;\n"
		"void main()\n"
		"{\n"
		"	fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\0";

	fragmentShader = NULL;
}

void FragmentShaderLoader::InitializeFragmentShaderLoader()
{
	// Compile the fragment shader using the shader object and the shader source code (GLSL)
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	int successfullyCompiled; // An integer that checks if the vertex shader compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	// Returns a successful compilation of the fragment shader if it can compile successfully
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successfullyCompiled);

	// If the compilation failed, then return a log compilation error and explain the error
	if (!successfullyCompiled)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << compilationInformationLog << std::endl;
	}
}