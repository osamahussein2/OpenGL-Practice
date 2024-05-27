#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>

class FragmentShaderLoader
{
public:
	FragmentShaderLoader();
	void InitializeFragmentShaderLoader();

	unsigned int fragmentShader;

private:
	const char* fragmentShaderSource;
};

