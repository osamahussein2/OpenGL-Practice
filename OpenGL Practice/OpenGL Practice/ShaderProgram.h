#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include "VertexShaderLoader.h"
#include "FragmentShaderLoader.h"

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	unsigned int transformMatrixLocation;

	glm::vec4 vector;
	glm::mat4 translateMatrix;
};

