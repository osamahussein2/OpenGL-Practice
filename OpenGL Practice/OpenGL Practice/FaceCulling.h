#pragma once

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glad/glad.h>
#include "stb_image.h"

#include <array>
#include <iostream>

using namespace glm;
using namespace std;

class FaceCulling
{
public:
	FaceCulling();
	~FaceCulling();

	void SetFaceCullingVertices();
private:
	unsigned int faceCullingVAO, faceCullingVBO;
	unsigned int faceCullingTexture;

	unsigned char* faceCullingData;

	array<float, 180> cubeVertices;

	int width, height, nrChannels;
};