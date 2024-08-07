#ifndef VERTEXSHADERLOADER_H
#define VERTEXSHADERLOADER_H

#include <glad/glad.h>
#include <glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "stb_image.h"

class VertexShaderLoader
{
public:
	VertexShaderLoader(const char* vertexShaderPath_);
	void InitializeVertexShaderLoader();
	void InitializeVertexObjects();
	void LoadThreeTriangles();

	unsigned int VAO, VBO;

	unsigned int vertexShader;

	// Unsigned variable types can only return positive values (or zero)
	// Signed variable types can return positive and negative values
	unsigned int texture1, texture2;

private:
	unsigned int EBO;
	std::string vertexShaderCode;
	std::ifstream vertexShaderFile;
	std::stringstream vertexShaderStream;
	const char* vShaderCode;
	//ShaderProgram shaderProg;
};

#endif