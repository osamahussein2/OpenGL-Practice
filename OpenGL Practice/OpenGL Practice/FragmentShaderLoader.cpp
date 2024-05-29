#include "FragmentShaderLoader.h"

FragmentShaderLoader::FragmentShaderLoader()
{
	fragmentShaderSource[0] = "#version 330 core\n"
		"out vec4 fragColor;\n"
		"\n"
		"in vec3 vertexColor; // Receives the vertex color we set inside the vertex shader and we can use it here\n"
		"\n"
		"void main()\n"
		"{\n"
		"	fragColor = vec4(vertexColor, 1.0f);\n"
		"}\0";

	fragmentShaderSource[1] = "#version 330 core\n"
		"out vec4 fragColor2;\n"
		"void main()\n"
		"{\n"
		"	fragColor2 = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
		"}\0";

	fragmentShaderSource[2] = "#version 330 core\n"
		"out vec4 fragColor3;\n"
		"void main()\n"
		"{\n"
		"	fragColor3 = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
		"}\0";

	size_t sizeOfFragmentShadersArray = sizeof(fragmentShader);

	for (int fragmentShaders = 0; fragmentShaders > sizeOfFragmentShadersArray; fragmentShaders++)
	{
		fragmentShader[fragmentShaders] = NULL;
	}
}

void FragmentShaderLoader::InitializeFragmentShaderLoader()
{
	// Compile the fragment shader using the shader object and the shader source code (GLSL)
	fragmentShader[0] = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader[0], 1, &fragmentShaderSource[0], NULL);
	glCompileShader(fragmentShader[0]);

	int successfullyCompiled; // An integer that checks if the vertex shader compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	// Returns a successful compilation of the fragment shader if it can compile successfully
	glGetShaderiv(fragmentShader[0], GL_COMPILE_STATUS, &successfullyCompiled);

	// If the compilation failed, then return a log compilation error and explain the error
	if (!successfullyCompiled)
	{
		glGetShaderInfoLog(fragmentShader[0], 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER1::FRAGMENT::COMPILATION_FAILED\n" << compilationInformationLog << std::endl;
	}
}

void FragmentShaderLoader::InitializeFragmentShaderLoader2()
{
	// Compile the fragment shader using the shader object and the shader source code (GLSL)
	fragmentShader[1] = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader[1], 1, &fragmentShaderSource[1], NULL);
	glCompileShader(fragmentShader[1]);

	int successfullyCompiled; // An integer that checks if the vertex shader compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	// Returns a successful compilation of the fragment shader if it can compile successfully
	glGetShaderiv(fragmentShader[1], GL_COMPILE_STATUS, &successfullyCompiled);

	// If the compilation failed, then return a log compilation error and explain the error
	if (!successfullyCompiled)
	{
		glGetShaderInfoLog(fragmentShader[1], 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER2::FRAGMENT::COMPILATION_FAILED\n" << compilationInformationLog << std::endl;
	}
}

void FragmentShaderLoader::InitializeFragmentShaderLoader3()
{
	// Compile the fragment shader using the shader object and the shader source code (GLSL)
	fragmentShader[2] = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader[2], 1, &fragmentShaderSource[2], NULL);
	glCompileShader(fragmentShader[2]);

	int successfullyCompiled; // An integer that checks if the vertex shader compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	// Returns a successful compilation of the fragment shader if it can compile successfully
	glGetShaderiv(fragmentShader[2], GL_COMPILE_STATUS, &successfullyCompiled);

	// If the compilation failed, then return a log compilation error and explain the error
	if (!successfullyCompiled)
	{
		glGetShaderInfoLog(fragmentShader[2], 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER3::FRAGMENT::COMPILATION_FAILED\n" << compilationInformationLog << std::endl;
	}
}
