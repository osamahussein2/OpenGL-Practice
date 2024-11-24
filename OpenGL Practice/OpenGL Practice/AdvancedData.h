#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <array>
#include "stb_image.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

class AdvancedData
{
public:
	AdvancedData();
	~AdvancedData();

	void MapBuffer();
private:
	array<float, 3> someData;
	void* pointer;
	int buffer;

	array<float, 3> positions;
	array<float, 3> normals;
	array<float, 2> textureCoordinates;
};