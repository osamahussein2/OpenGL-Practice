#version 330 core

layout (location = 0) in vec3 position;

out vec3 texCoords;

uniform mat4 skyboxModelMatrix, skyboxProjectionMatrix, skyboxViewMatrix;

void main()
{
	texCoords = position;
	vec4 skyboxPos = skyboxProjectionMatrix * skyboxModelMatrix * skyboxModelMatrix * vec4(position, 1.0);
	gl_Position = skyboxPos.xyww;
}