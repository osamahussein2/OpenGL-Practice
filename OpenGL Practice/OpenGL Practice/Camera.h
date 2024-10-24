#pragma once

#include <glfw3.h>

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

static enum CameraMovement
{
	MoveCameraForward,
	MoveCameraBackward,
	MoveCameraLeft,
	MoveCameraRight
};

class Camera
{
public:

	// Always create a constructor and deconstructor for every class to declare the class' objects and variables
	Camera();
	~Camera();

	void InitializeCamera();

	static glm::mat4 CameraLookAt();

	// Create static instances of camera vec3s so that we can easily use them inside of another class
	static glm::vec3 cameraPosition, cameraTarget, cameraDirection;
	static glm::vec3 cameraUp, cameraRight, cameraFront;
	static glm::vec3 Xaxis, Yaxis, Zaxis;

	static float yaw;
	static float pitch;

	static float fieldOfView;

	const float radius = 10.0f;
	float cameraX, cameraZ;

	static bool constrainPitch;
};