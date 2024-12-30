#pragma once

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <array>

#include "ShaderProgram.h"

using namespace std;
using namespace glm;

class AntiAliasing
{
public:
	AntiAliasing();
	~AntiAliasing();

	void InitializeAntiAliasing();
	void RenderAntiAliasing();

private:
	ShaderProgram* antiAliasingShaderProgram;
	ShaderProgram* antiAliasingPostShaderProgram;

	array<float, 108> cubeVertices;
	unsigned int cubeVAO, cubeVBO;

	array<float, 24> quadVertices;
	unsigned int quadVAO, quadVBO;

	unsigned int framebuffer;
	unsigned int textureColorBufferMultiSampled;
	unsigned int RBO;

	unsigned int intermediateFBO;
	unsigned int screenTexture;

};