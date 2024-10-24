#include "Camera.h"

// ALWAYS declare the static variables and objects above the class functions
float Camera::yaw = -90.0f;
float Camera::pitch = 0.0f;

glm::vec3 Camera::cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 Camera::cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);

// The camera position is a vector in world space that points to the camera's position
glm::vec3 Camera::cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);

glm::vec3 Camera::cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 Camera::cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 Camera::cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);

float Camera::fieldOfView = 45.0f;

glm::vec3 Camera::Zaxis = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 Camera::Xaxis = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 Camera::Yaxis = glm::vec3(0.0f, 0.0f, 0.0f);

bool Camera::constrainPitch = true;

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

glm::mat4 Camera::CameraLookAt()
{
	// Calculate the camera's direction using the camera's known position vector
	Zaxis = glm::normalize(cameraPosition - (cameraPosition + cameraFront));

	// Get the positive right vector axis
	Xaxis = glm::normalize(glm::cross(glm::normalize(cameraUp), Zaxis));

	// Calculate the camera up vector
	Yaxis = glm::cross(Zaxis, Xaxis);

	glm::mat4 translationMatrix = glm::mat4(1.0f); // Set the translation matrix equal to the identity matrix

	translationMatrix[3][0] = -cameraPosition.x; // 4th column of the matrix and 1st row
	translationMatrix[3][1] = -cameraPosition.y; // 4th column of the matrix and 2nd row
	translationMatrix[3][2] = -cameraPosition.z; // 4th column of the matrix and 3rd row

	glm::mat4 rotationMatrix = glm::mat4(1.0f); // Set the rotation matrix equal to the identity matrix

	rotationMatrix[0][0] = Xaxis.x; // 1st column of the matrix and 1st row
	rotationMatrix[1][0] = Xaxis.y; // 2nd column of the matrix and 1st row
	rotationMatrix[2][0] = Xaxis.z; // 3rd column of the matrix and 1st row

	rotationMatrix[0][1] = Yaxis.x; // 1st column of the matrix and 2nd row
	rotationMatrix[1][1] = Yaxis.y; // 2nd column of the matrix and 2nd row
	rotationMatrix[2][1] = Yaxis.z; // 3rd column of the matrix and 2nd row

	rotationMatrix[0][2] = Zaxis.x; // 1st column of the matrix and 3rd row
	rotationMatrix[1][2] = Zaxis.y; // 2nd column of the matrix and 3rd row
	rotationMatrix[2][2] = Zaxis.z; // 3rd column of the matrix and 3rd row

	return rotationMatrix * translationMatrix; // Read from right to left (first translate then rotate)
}