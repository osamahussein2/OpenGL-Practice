#version 330 core

out vec4 fragColor;

in vec3 environmentMappingNormal;
in vec3 environmentMappingPosition;

uniform vec3 cameraPosition;
uniform samplerCube cubeMap;

void main()
{
	vec3 viewDirection = normalize(environmentMappingPosition - cameraPosition);
	vec3 reflectiveVector = reflect(viewDirection, normalize(environmentMappingNormal));

	fragColor = vec4(vec3(texture(cubeMap, reflectiveVector)), 1.0);
}