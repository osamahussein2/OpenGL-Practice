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

class ShadowMapping
{
public:
	ShadowMapping();
	~ShadowMapping();

	static ShadowMapping* Instance();

	void InitializePlaneVertices();
	void InitializeTexture(char const* path);
	void InitializeFramebuffers();

	void UseShaderProgram();

	void RenderScene(ShaderProgram* shaderProgram_);
	void RenderCube();
	void RenderQuad();

private:
	static ShadowMapping* instance;

	unsigned int depthMapFBO;

	// Resolution of depth map
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMap;

	array<ShaderProgram*, 3> shadowMappingShaderProgram;

	unsigned int planeVAO, planeVBO;

	array<float, 48> planeVertices;

	struct VertexAttributes
	{
		const int positionAttribute = 0, normalAttribute = 1, textureAttribute = 2;
	};

	VertexAttributes vertexAttributes;

	unsigned int woodTexture;

	glm::vec3 lightPosition;
};

typedef ShadowMapping TheShadowMapping;