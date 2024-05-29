#ifndef VERTEXSHADERLOADER_H
#define VERTEXSHADERLOADER_H

#include <glad/glad.h>
#include <glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>

class VertexShaderLoader
{
public:
	VertexShaderLoader(const char* vertexShaderPath_);
	void InitializeVertexShaderLoader();
	void InitializeVertexObjects();
	void LoadThreeTriangles();

	unsigned int vertexShader;

private:
	unsigned int VAO, VBO;
	unsigned int EBO;
	std::string vertexShaderCode;
	std::ifstream vertexShaderFile;
	std::stringstream vertexShaderStream;
	const char* vShaderCode;
};

#endif