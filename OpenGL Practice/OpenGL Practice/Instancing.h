#pragma once

#include <array>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

#include "Model.h"

class Instancing
{
public:
	Instancing();
	~Instancing();

	void SetInstancingOffsetPositions();
	void InitializeInstancingVertices();

	void SetTransformationMatrix();

	void SetInstancedArrays();

	void UseInstancingShaderProgram();

private:
	array<float, 30> quadVertices;

	unsigned int quadVAO, quadVBO;
	unsigned int instanceVBO;

	unsigned int buffer;

	vec2 translations[100];

	ShaderProgram* modelShaderProgram;
	ShaderProgram* instancingShaderProgram;

	Model* rock;
	Model* planet;

	unsigned int amount;
	glm::mat4* modelMatrices;
};