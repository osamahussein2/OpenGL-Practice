#ifndef FRAGMENTSHADERLOADER_H
#define FRAGMENTSHADERLOADER_H

#include <glad/glad.h>
#include <glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>

class FragmentShaderLoader
{
public:
	FragmentShaderLoader(const char* fragmentShaderPath_);
	void InitializeFragmentShaderLoader();

	unsigned int fragmentShader;

private:
	std::string fragmentShaderCode;
	std::ifstream fragmentShaderFile;
	std::stringstream fragmentShaderStream;
	const char* fShaderCode;
};

#endif