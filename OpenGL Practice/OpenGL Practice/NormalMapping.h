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

class NormalMapping
{
public:
	~NormalMapping();

	static NormalMapping* Instance();

	void InitializeNormalMapping();
	void RenderNormalMapping();

private:
	NormalMapping();

	unsigned int LoadTexture(char const* path_);

	void RenderQuad();

	static NormalMapping* normalMappingInstance;

	ShaderProgram* normalMappingShaderProgram;

	unsigned int diffuseMap;
	unsigned int normalMap;

	vec3 lightPosition;

	unsigned int quadVAO;
	unsigned int quadVBO;
};