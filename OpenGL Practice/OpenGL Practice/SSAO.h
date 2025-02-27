#pragma once

#include <array>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "ShaderProgram.h"
#include "Model.h"

class SSAO
{
public:
	~SSAO();

	static SSAO* Instance();

	void InitializeSSAO();
	void RenderSSAO();

private:
	SSAO();

	void RenderQuad();
	void RenderCube();

	static SSAO* ssaoInstance;

	unsigned int cubeVAO, cubeVBO, quadVAO, quadVBO;

	array<ShaderProgram*, 4> ssaoShaders;

	Model* backpack;

	unsigned int gBuffer, gPosition, gNormal, gAlbedo, rboDepth;

	array<unsigned int, 3> attachments;

	unsigned int ssaoFBO, ssaoBlurFBO, ssaoColorBuffer, ssaoColorBufferBlur;

	vector<vec3> ssaoKernel, ssaoNoise;

	unsigned int noiseTexture;

	vec3 lightPos;
	vec3 lightColor;
};