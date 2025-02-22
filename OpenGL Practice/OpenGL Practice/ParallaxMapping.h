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

class ParallaxMapping
{
public:
	~ParallaxMapping();

	static ParallaxMapping* Instance();

	void InitializeParallaxMapping();
	void RenderParallaxMapping();

	float heightScale;

private:
	ParallaxMapping();

	unsigned int LoadTexture(char const* path_);

	void RenderQuad();

	static ParallaxMapping* parallaxMappingInstance;

	ShaderProgram* parallaxMappingShaderProgram;

	unsigned int diffuseMap, normalMap, heightMap;

	vec3 lightPosition;

	unsigned int quadVAO, quadVBO;
};