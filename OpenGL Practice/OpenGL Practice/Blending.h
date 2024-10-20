#pragma once


// Include the GLM header files (OpenGL Mathematics Library)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>

#include "stb_image.h"

#include <vector>
#include <map>

#include "ShaderProgram.h"
#include "Camera.h"

using namespace std;
using namespace glm;

class Blending
{
public:
	Blending();
	~Blending();

	void SetBlending();
	void IncludeGrassBlending();
	void IncludeTransparentWindowBlending();
private:
	vector<vec3> vegetation;
	vector<vec3> windows;

	unsigned int transparentVAO, transparentVBO;
	unsigned int transparentTexture;

	unsigned char* transparentData;

	int width, height, nrChannels;

	mat4 blendingModelMatrix;
};