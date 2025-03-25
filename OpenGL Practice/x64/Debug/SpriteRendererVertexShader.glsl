#version 330 core

// This also works but we need to use the floating components though to set the position and texture coordinate data
// layout (location = 0) in vec4 vertex; // one for vec2 position and the other, vec2 textureCoords

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 textureCoords;

out vec2 texCoords;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

void main()
{
	texCoords = textureCoords;
	gl_Position = projectionMatrix * modelMatrix * vec4(position, 0.0, 1.0);
}