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

class GammaCorrection
{
public:
	GammaCorrection();
	~GammaCorrection();

	static GammaCorrection* Instance();

	void InitializeGamma();
	void InitializeTextures(char const* path_, bool gammaCorrection_);
	void InitializeGammaCorrectedTextures(char const* path_, bool gammaCorrection_);
	void InitializeLighting();

	void UseGammaShaderProgram();

	bool gammaEnabled;

private:
	static GammaCorrection* gammaCorrectionInstance;

	enum VertexAttributes
	{
		POSITION_ATTRIBUTE = 0,
		NORMAL_ATTRIBUTE = 1,
		TEXTURE_COORDINATE_ATTRIBUTE = 2
	};

	array<float, 48> planeVertices;

	ShaderProgram* gammaCorrectionShaderProgram;

	unsigned int floorTexture;
	unsigned int floorTextureGammaCorrected;

	unsigned int planeVAO, planeVBO;

	array<vec3, 4> lightPositions;
	array<vec3, 4> lightColors;
};