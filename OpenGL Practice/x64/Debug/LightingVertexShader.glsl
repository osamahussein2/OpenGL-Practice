#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

// Send the fragment position and the normal to the fragment shader so that we can set them there
out vec3 FragPosition;
out vec3 Normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

	// Multiply the fragment position using the model matrix to transform it to world space coordinates
	FragPosition = vec3(modelMatrix * vec4(position, 1.0));

	// Generate the normal matrix by using the transpose and inverse functions (only if the vec3 is non-uniform scale)
	// The matrix has to be casted as a mat3 to make sure it doesn't have translation properties anymore
	Normal =  mat3(transpose(inverse(modelMatrix))) * normal;
}