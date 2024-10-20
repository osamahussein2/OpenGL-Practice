#pragma once


// Include the GLM header files (OpenGL Mathematics Library)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>

#include "stb_image.h"

#include <vector>

#include "ShaderProgram.h"

using namespace std;
using namespace glm;

class Blending
{
public:
	Blending();
	~Blending();

	void SetBlending();
	void UseShaderProgramForBlending();
private:
	vector<vec3> vegetation;

	unsigned int vegetationVAO, vegetationVBO;
	unsigned int vegetationTexture;

	unsigned char* vegetationData;

	int width, height, nrChannels;

	mat4 blendingModelMatrix;
};