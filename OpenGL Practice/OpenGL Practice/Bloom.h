#pragma once

#include <array>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "ShaderProgram.h"

using namespace std;
using namespace glm;

class Bloom
{
public:
	~Bloom();

	static Bloom* Instance();

	void InitializeBloom();
	void RenderBloom();

	bool bloom;
	float exposure;

private:
	Bloom();

	unsigned int LoadTexture(const char* path, bool gammaCorrection);

	void RenderCube();
	void RenderQuad();

	static Bloom* bloomInstance;

	unsigned int hdrFBO, rboDepth;

	unsigned int colorBuffers[2], attachments[2], pingpongFBO[2], pingpongColorbuffers[2];

	array<ShaderProgram*, 4> bloomShaders;

	unsigned int woodTexture, containerTexture;

	vector<vec3> lightPositions, lightColors;

	unsigned int cubeVAO, cubeVBO, quadVAO, quadVBO;
};