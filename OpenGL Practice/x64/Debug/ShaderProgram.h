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
	void InitializeSecondTexture();
	void Initialize3Dobjects(float aspect_ratio, float near_plane, float far_plane);

	float visibilityTextureValue;

private:
	VertexShaderLoader* vertexShaderLoader;
	FragmentShaderLoader* fragmentShaderLoader;

	unsigned int shaderProgram;
	float timer;
	float moveRight;
	int movingPositionLocation;
	unsigned int transformMatrixLocation;
	
	int modelMatrixLocation;
	int viewMatrixLocation;
	int projectionMatrixLocation;

	glm::vec4 vector;
	glm::mat4 translateMatrix;

	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	std::array<glm::vec3, 10> cubePositions;
};

