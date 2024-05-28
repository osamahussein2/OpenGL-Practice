#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>

class FragmentShaderLoader
{
public:
	FragmentShaderLoader();
	void InitializeFragmentShaderLoader();
	void InitializeFragmentShaderLoader2();
	void InitializeFragmentShaderLoader3();

	unsigned int fragmentShader[3];

private:
	const char* fragmentShaderSource[3];
};

