#include "Camera.h"

// ALWAYS declare the static variables and objects above the class functions
glm::vec3 Camera::cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 Camera::cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);

// The camera position is a vector in world space that points to the camera's position
glm::vec3 Camera::cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);

glm::vec3 Camera::cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 Camera::cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 Camera::cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);

Camera::Camera()
{
	cameraX = 0.0f;
	cameraZ = 0.0f;
}

Camera::~Camera()
{

}

void Camera::InitializeCamera()
{
	// The next vector required is the camera's direction, or what direction the camera is pointing at
	// Subtracting the camera position vector from the scene's origin vector results in the direction vector we want

	/* For the view matrix's coordinate system we want its z-axis to be positive and because by convention (in OpenGL)
	the camera points towards the negative z-axis we want to negate (undo) the direction vector. */

	// To get a vector pointing towards the camera's positive z-axis, switch the subtraction order around
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraDirection = glm::normalize(cameraPosition - cameraTarget);

	// We will then need a right direction vector that represents the positive x-axis of the camera space

	/* To get the right direction vector, first specify an up vector that points upwards (in world space). Then,
	use cross product on the up vector and the direction vector. Since the result of a cross product is a vector
	perpendicular to both vectors, we will get a vector that points in the positive x-axis's direction. */
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(up, cameraDirection));

	/* To get the vector that points to the camera's positive y-axis, we must perform a cross product with the
	right and direction vector. */
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));

	cameraX = sin(glfwGetTime()) * radius;
	cameraZ = sin(glfwGetTime()) * radius;
}