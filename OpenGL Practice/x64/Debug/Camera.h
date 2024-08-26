#pragma once

#include <glfw3.h>

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
public:

	// Always create a constructor and deconstructor for every class to declare the class' objects and variables
	Camera();
	~Camera();

	void InitializeCamera();

	// Create static instances of camera vec3s so that we can easily use them inside of another class
	static glm::vec3 cameraPosition, cameraTarget, cameraDirection;
	static glm::vec3 cameraUp, cameraRight, cameraFront;

	static float yaw;
	static float pitch;

	static float fieldOfView;

	const float radius = 10.0f;
	float cameraX, cameraZ;

private:

};