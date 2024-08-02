#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>

class Texture
{
public:
	Texture();
	~Texture();

	void InitializeTexture();

private:
	//std::array<float, 9> textureCoordinates;
};