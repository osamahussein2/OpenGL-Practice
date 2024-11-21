#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <array>
#include "stb_image.h"

using namespace std;

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
};