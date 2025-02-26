#pragma once

#include <array>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "ShaderProgram.h"
#include "Model.h"

using namespace std;
using namespace glm;

class DeferredShading
{
public:
	~DeferredShading();

	static DeferredShading* Instance();

	void InitializeDeferredShading();
	void RenderDeferredShading();

private:
	DeferredShading();

	void RenderQuad();
	void RenderCube();

	static DeferredShading* deferredShadingInstance;

	array<ShaderProgram*, 3> deferredShadings;

	unsigned int quadVAO, quadVBO, cubeVAO, cubeVBO;

	Model* backpack;

	vector<vec3> objectPositions;

	unsigned int gBuffer, gPosition, gNormal, gAlbedoSpec, rboDepth;

	const unsigned int NR_LIGHTS = 32;

	vector<vec3> lightPositions;
	vector<vec3> lightColors;

	array<unsigned int, 3> attachments;

	// We don't need to send this to the shader, we assume it is always 1.0 (in our case)
	const float constant = 1.0f;

	const float linear = 0.7f;
	const float quadratic = 1.8f;
};