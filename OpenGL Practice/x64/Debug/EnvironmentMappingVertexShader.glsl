#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

// The Normal output here is used to normalize the normals in the reflective vector inside the fragment shader
out vec3 environmentMappingNormal;

// The Position output here is used to calculate the view direction vector in the fragment shader
out vec3 environmentMappingPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;


void main()
{
	environmentMappingNormal = mat3(transpose(inverse(modelMatrix))) * normal;
	environmentMappingPosition = vec3(modelMatrix * vec4(position, 1.0));

	gl_Position = projectionMatrix * viewMatrix * vec4(environmentMappingPosition, 1.0);
}