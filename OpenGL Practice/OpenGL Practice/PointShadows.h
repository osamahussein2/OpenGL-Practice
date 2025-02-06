#pragma once

#include <array>
#include <vector>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "ShaderProgram.h"

using namespace std;
using namespace glm;

class PointShadows
{
public:
	~PointShadows();

	static PointShadows* Instance();

	void InitializePointShadows();
	void InitializeTexture(const char* path_);
	void InitializeDepthCubemapTexture();
	void InitializeFramebuffers();
	void InitializeTextureUniformShaders();

	void ShowPointShadows();

	bool shadows;
	
private:
	PointShadows();

	void RenderScene(ShaderProgram* shaderProgram_);
	void RenderCube();

	static PointShadows* pointShadowsInstance;

	array<ShaderProgram*, 2> pointShadowShaderProgram;

	array<VertexShaderLoader*, 2> vertexShaderLoader;
	array<FragmentShaderLoader*, 2> fragmentShaderLoader;

	GeometryShader* geometryShaderLoader;

	unsigned int woodTexture;

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;

	unsigned int depthCubemap;

	vec3 lightPosition;

	unsigned int cubeVAO, cubeVBO;

	enum VertexAttributes
	{
		POSITION = 0,
		NORMAL = 1,
		TEXTURE = 2
	};
};

typedef PointShadows ThePointShadows;