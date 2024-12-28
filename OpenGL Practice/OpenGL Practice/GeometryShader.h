#pragma once

#include <array>
#include <glad/glad.h>
#include <glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "stb_image.h"

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>


using namespace std;

class GeometryShader
{
public:
	GeometryShader(const char* geometryShaderPath_);
	~GeometryShader();

	void InitializeGeometryShaderLoader();

	void InitializeGeometryModel();

	void InitializeGeometryVertices();
	void DrawGeometryPoints();
	void DrawExplodingModel();

	void DrawVisualizingNormalVectors();

	unsigned int geometryShader;

private:
	//array<float, 20> points;

	unsigned int pointsVBO, pointsVAO;

	string geometryShaderCode;
	ifstream geometryShaderFile;
	stringstream geometryShaderStream;
	const char* gShaderCode;
};