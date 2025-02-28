#pragma once

#include <array>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "ShaderProgram.h"
#include "Model.h"

class PBRLighting
{
public:
	~PBRLighting();

	static PBRLighting* Instance();

	void InitializePBRLighting();
	void RenderPBRLighting();

private:
	PBRLighting();

	unsigned int LoadTexture(const char* path);
	void RenderSphere();

	static PBRLighting* pbrLightingInstance;

	ShaderProgram* pbrLightingShader;

	array<vec3, 1> lightPositions;
	array<vec3, 1> lightColors;

	int nrRows;
	int nrColumns;
	float spacing;

	mat4 projectionMatrix, viewMatrix, modelMatrix;

	unsigned int sphereVAO;
	unsigned int indexCount;

	unsigned int albedo, normal, metallic, roughness, ambientOcclusion;
};