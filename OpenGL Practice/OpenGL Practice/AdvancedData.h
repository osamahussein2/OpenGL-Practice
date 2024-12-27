#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <array>
#include "stb_image.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "ShaderProgram.h"
#include "Camera.h"

using namespace std;
using namespace glm;

class AdvancedData
{
public:
	AdvancedData();
	~AdvancedData();

	void InitializeCubeVertices();
	void InitializeBufferObject();

	void LoadCubesInfo();

	void DrawRedCube();
	void DrawGreenCube();
	void DrawBlueCube();
	void DrawYellowCube();

	//void MapBuffer(); // I created this for learning purposes not for getting it working
private:
	/*array<float, 3> someData;
	void* pointer;
	int buffer;*/

	mat4 modelMatrix;

	/*array<float, 3> positions;
	array<float, 3> normals;
	array<float, 2> textureCoordinates;*/

	unsigned int cubeVAO, cubeVBO;
	unsigned int uboMatrices;

	array<float, 216> cubeVertices;

	ShaderProgram* redShader;
	ShaderProgram* greenShader;
	ShaderProgram* blueShader;
	ShaderProgram* yellowShader;
};