#ifndef VERTEXSHADERLOADER_H
#define VERTEXSHADERLOADER_H

#include <glad/glad.h>
#include <glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>
#include <array>
#include "stb_image.h"

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class VertexShaderLoader
{
public:
	VertexShaderLoader(const char* vertexShaderPath_);
	void InitializeVertexShaderLoader();
	void InitializeVertexObjects();
	//void InitializeLightColorVertexObjects();

	unsigned int vertexShader;

private:
	unsigned int VAO, VBO, lightVAO;
	//unsigned int EBO;
	std::string vertexShaderCode;
	std::ifstream vertexShaderFile;
	std::stringstream vertexShaderStream;
	const char* vShaderCode;
	unsigned char* data;

	int width, height, nrChannels;

	// Unsigned variable types can only return positive values (or zero)
	// Signed variable types can return positive and negative values
	unsigned int texture1, texture2;

	std::array<float, 108> vertices;
	//std::array<unsigned int, 6> indices;

	// I tried to create vertices' and indices' variables as std::lists and the triangles wouldn't render at all
	
	//std::list<float> vertices;
	//std::list<unsigned int> indices;
};

#endif