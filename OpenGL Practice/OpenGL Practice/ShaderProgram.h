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
	void InitializeShaderProgram2();
	void InitializeShaderProgram3();

private:
	VertexShaderLoader vertexShaderLoader;
	FragmentShaderLoader fragmentShaderLoader;

	unsigned int shaderProgram[3];
};

