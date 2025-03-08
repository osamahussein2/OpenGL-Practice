#pragma once

#include <array>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "stb_image.h"

#include "ShaderProgram.h"

class SpecularIBL
{
public:
	~SpecularIBL();

	static SpecularIBL* Instance();

	void InitializeSpecularIBL();
	void RenderSpecularIBL();

private:
	SpecularIBL();

	unsigned int LoadTexture(const char* path);
	void RenderSphere();
	void RenderCube();
	void RenderQuad();

	static SpecularIBL* specularIBLinstance;

	array<ShaderProgram*, 6> specularIBLshaders;

	array<vec3, 4> lightPositions, lightColors;

	int nrRows, nrColumns;
	float spacing;

	unsigned int captureFBO, captureRBO;

	float* data;
	unsigned int hdrTexture;

	unsigned int envCubemap;

	mat4 captureProjection;
	array<mat4, 6> captureViews;

	unsigned int irradianceMap;

	unsigned int prefilterMap;

	unsigned int maxMipLevels;
	unsigned int brdfLUTTexture;

	mat4 projection;

	unsigned int sphereVAO, indexCount, cubeVAO, cubeVBO, quadVAO, quadVBO;

    unsigned int ironAlbedoMap, ironNormalMap, ironMetallicMap, ironRoughnessMap, ironAOMap;

	unsigned int goldAlbedoMap, goldNormalMap, goldMetallicMap, goldRoughnessMap, goldAOMap;

	unsigned int grassAlbedoMap, grassNormalMap, grassMetallicMap, grassRoughnessMap, grassAOMap;

	unsigned int plasticAlbedoMap, plasticNormalMap, plasticMetallicMap, plasticRoughnessMap, plasticAOMap;

	unsigned int wallAlbedoMap, wallNormalMap, wallMetallicMap, wallRoughnessMap, wallAOMap;
};