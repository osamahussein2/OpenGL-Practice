#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include "VertexShaderLoader.h"
#include "FragmentShaderLoader.h"
#include "GeometryShader.h"
#include "Color.h"
#include "Lighting.h"
#include "FrameBuffer.h"

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();
	void InitializeShaderProgram(VertexShaderLoader* vertexShader_, FragmentShaderLoader* fragmentShader_,
		GeometryShader* geometryShader_ = nullptr);

	void Compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource);

	void CheckCompileErrors(unsigned int object, string type);

	//void InitializeModeling(float aspect_ratio, float near_plane, float far_plane);
	//void InitializeCubeDepthTesting(float aspect_ratio, float near_plane, float far_plane);
	//void InitializeScaledCubeStencilTesting(float aspect_ratio, float near_plane, float far_plane);
	//void InitializeFloorDepthTesting();
	//void InitializeSecondTexture();
	//void InitializeCubeColor(float aspect_ratio, float near_plane, float far_plane);
	//void InitializeLightColor(float aspect_ratio, float near_plane, float far_plane);
	//void CreateDesertLighting();
	//void CreateFactoryLighting();
	//void CreateHorrorLighting();
	//void CreateBioChemicalLabLighting();

	void DeleteShaders(VertexShaderLoader* vertexShader_, FragmentShaderLoader* fragmentShader_, 
		GeometryShader* geometryShader_);

	float visibilityTextureValue;

	unsigned int shaderProgram;

private:
	Color* color;
	Lighting* lighting;

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
	std::array<glm::vec3, 4> pointLightPositions;
};

