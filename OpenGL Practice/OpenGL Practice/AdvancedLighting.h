#pragma once

#include <array>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "ShaderProgram.h"

using namespace std;

class AdvancedLighting
{
public:
	~AdvancedLighting();

	static AdvancedLighting* Instance();

	void InitializeVertices();
	void InitializeTextures();

	void SetUpAdvancedLighting();

	bool isBlinnOn;

private:
	AdvancedLighting();

	static AdvancedLighting* advancedLightingInstance;

	array<float, 48> planeVertices;

	ShaderProgram* advancedLightingShaderProgram;

	unsigned int floorTexture;

	unsigned int planeVAO, planeVBO;

	vec3 lightPosition;
};

/* typedef stands for type definition. C++ typedef helps giving something (e.g. class) a new name to an existing data type. 
Basically, it's a reserved keyword to create a name for a specific data type. The function or purpose served by the data type 
remains unchanged (learned this during one of my SDL practice runs in C++) */
typedef AdvancedLighting TheAdvancedLighting;