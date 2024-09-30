#version 330 core

layout (location = 0) in vec3 modelPosition;
layout (location = 1) in vec3 modelNormal;
layout (location = 2) in vec2 modelTextureCoordinate;

out vec2 textureCoords;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
	textureCoords = modelTextureCoordinate;

	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(modelPosition, 1.0);
}