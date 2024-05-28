#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>

class VertexShaderLoader
{
public:
	VertexShaderLoader();
	void InitializeVertexShaderLoader();
	void InitializeVertexShaderLoader2();
	void InitializeVertexShaderLoader3();
	void InitializeVertexObjects();
	void LoadThreeTriangles();

	unsigned int vertexShader[3];

private:
	const char* vertexShaderSource[3];
	unsigned int VAO, VBO;
	unsigned int EBO;
};

