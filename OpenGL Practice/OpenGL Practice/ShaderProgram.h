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

	float visibilityTextureValue;

private:
	VertexShaderLoader* vertexShaderLoader;
	FragmentShaderLoader* fragmentShaderLoader;

	unsigned int shaderProgram;
	float timer;
	float moveRight;
	int movingPositionLocation;
};

