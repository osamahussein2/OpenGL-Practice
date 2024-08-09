#include "FragmentShaderLoader.h"

FragmentShaderLoader::FragmentShaderLoader(const char* fragmentShaderPath_)
{
	// Make sure the fragment shader file ifstream will throw an exception error
	fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// Open the fragment shader file
		fragmentShaderFile.open(fragmentShaderPath_);

		// Read the fragment shader's file buffer contents into a stream of the fragment shader file
		fragmentShaderStream << fragmentShaderFile.rdbuf();

		// Close the fragment shader's file handlers
		fragmentShaderFile.close();

		// Convert the fragment shader stream into a fragment shader string
		fragmentShaderCode = fragmentShaderStream.str();
	}

	catch (std::ifstream::failure error)
	{
		std::cout << "ERROR::SHADER::FRAGMENT::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}

	fShaderCode = fragmentShaderCode.c_str();
}

void FragmentShaderLoader::InitializeFragmentShaderLoader()
{
	// Compile the fragment shader using the shader object and the shader source code (GLSL)
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
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