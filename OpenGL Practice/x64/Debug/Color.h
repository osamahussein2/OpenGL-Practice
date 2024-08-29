#pragma once

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Color
{
public:
	Color();
	~Color();

	void InitializeColor();

	glm::vec3 coral, lightColor, toyColor, resultingColor;
private:
	
};