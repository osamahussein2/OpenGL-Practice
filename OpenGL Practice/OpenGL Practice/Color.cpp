#include "Color.h"

Color::Color()
{
	coral = glm::vec3(0.0f, 0.0f, 0.0f);
	lightColor = glm::vec3(0.0f, 0.0f, 0.0f);
	toyColor = glm::vec3(0.0f, 0.0f, 0.0f);
	resultingColor = glm::vec3(0.0f, 0.0f, 0.0f);
}

Color::~Color()
{
	coral = glm::vec3(0.0f, 0.0f, 0.0f);
	lightColor = glm::vec3(0.0f, 0.0f, 0.0f);
	toyColor = glm::vec3(0.0f, 0.0f, 0.0f);
	resultingColor = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Color::InitializeColor()
{
	// Colors are digitally represented using a red, green and blue component, commonly abbrevaiated as RGB.
	coral = glm::vec3(1.0f, 0.5f, 0.31f);

	// It's important that when defining a light source in OpenGL, the light source must have a color using a vec3
	lightColor = glm::vec3(0.33f, 0.42f, 0.18f); // Dark olive-green color

	// Object's color 
	toyColor = coral;

	/* If we multiply the light source's color with an object's color value, the resulting color would be the reflected
	color of the object (perceived color). */
	resultingColor = lightColor * toyColor;
}