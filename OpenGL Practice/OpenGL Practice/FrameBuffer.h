#pragma once

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glad/glad.h>
#include "stb_image.h"

#include <array>
#include <iostream>
#include <vector>

#include "ShaderProgram.h"

using namespace glm;
using namespace std;

class FrameBuffer
{
public:
	FrameBuffer();
	~FrameBuffer();

	void InitializeCubeVertices();
	void InitializePlaneVertices();
	void InitializeQuadVertices();

	void AddFrameBuffer();

	void RenderScene();

	void InitializeCubeTextures();
	void InitializePlaneTextures();
	void InitializeQuadTextures();

	void BindToDefaultFrameBuffer();

private:
	unsigned int frameBufferFBO, frameBufferRBO;

	unsigned int cubeVAO, cubeVBO;
	unsigned int planeVAO, planeVBO;
	unsigned int quadVAO, quadVBO;

	unsigned int frameBufferTexture, cubeTexture, planeTexture;

	unsigned char* frameBufferData;

	array<float, 180> cubeVertices;
	array<float, 30> planeVertices;
	array<float, 30> quadVertices;

	int width, height, nrChannels;
};