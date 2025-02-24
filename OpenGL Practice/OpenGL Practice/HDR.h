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

class HDR
{
public:
	~HDR();

	static HDR* Instance();

	bool hdr;
	float exposure;

	void InitializeHDR();
	void RenderHDR();

private:
	HDR();
	
	unsigned int LoadTexture(const char* path_, bool gammaCorrection);

	void RenderCube();
	void RenderQuad();

	static HDR* hdrInstance;

	array<ShaderProgram*, 2> shaders;

	unsigned int woodTexture;

	unsigned int hdrFBO, colorBuffer, rboDepth;

	vector<glm::vec3> lightPositions;
	vector<glm::vec3> lightColors;

	unsigned int cubeVAO;
	unsigned int cubeVBO;

	unsigned int quadVAO;
	unsigned int quadVBO;
};