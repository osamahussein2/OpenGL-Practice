#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>

class VertexShaderLoader
{
public:
	VertexShaderLoader();
	void InitializeVertexShaderLoader();
	void InitializeVertexObjects();

	unsigned int vertexShader;

private:
	const char* vertexShaderSource;
	unsigned int VAO, VBO;
	unsigned int EBO;
};

