#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include "VertexShaderLoader.h"
#include "FragmentShaderLoader.h"

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();
	void InitializeShaderProgram();

	unsigned int shaderProgram;

private:
	VertexShaderLoader vertexShaderLoader;
	FragmentShaderLoader fragmentShaderLoader;
};

