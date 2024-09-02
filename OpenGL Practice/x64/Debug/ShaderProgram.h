#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include "VertexShaderLoader.h"
#include "FragmentShaderLoader.h"
#include "Color.h"

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();
	void InitializeShaderProgram(VertexShaderLoader* vertexShader_, FragmentShaderLoader* fragmentShader_);
	void InitializeSecondTexture();
	void InitializeCubeColor(float aspect_ratio, float near_plane, float far_plane);
	void InitializeLightColor(float aspect_ratio, float near_plane, float far_plane);

	float visibilityTextureValue;

private:
	std::array<VertexShaderLoader*, 2> vertexShaderLoader;
	std::array<FragmentShaderLoader*, 2> fragmentShaderLoader;
	Color* color;

	unsigned int shaderProgram;
	float timer;
	float moveRight;
	int movingPositionLocation;
	unsigned int transformMatrixLocation;

	unsigned int objectColorLocation;
	unsigned int lightColorLocation;
	
	int modelMatrixLocation;
	int viewMatrixLocation;
	int projectionMatrixLocation;

	int lightPositionLocation;
	int viewPositionLocation;

	glm::vec4 vector;
	glm::mat4 translateMatrix;

	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	glm::vec3 lightPosition;

	std::array<glm::vec3, 10> cubePositions;
};

