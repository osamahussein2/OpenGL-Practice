#version 330 core

out vec4 fragColor;

in vec3 texCoords;

uniform samplerCube cubeMap; // Cube map texture sampler

void main()
{
	fragColor = texture(cubeMap, texCoords);
}