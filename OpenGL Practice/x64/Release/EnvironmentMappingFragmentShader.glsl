#version 330 core

out vec4 fragColor;

in vec3 environmentMappingNormal;
in vec3 environmentMappingPosition;

uniform vec3 cameraPosition;
uniform samplerCube cubeMap;

uniform float refractionRatio;

vec4 reflectionMapping()
{
	vec3 viewDirection = normalize(environmentMappingPosition - cameraPosition);
	vec3 reflectiveVector = reflect(viewDirection, normalize(environmentMappingNormal));

	return vec4(vec3(texture(cubeMap, reflectiveVector)), 1.0);
}

vec4 refractionMapping()
{
	vec3 viewDirection = normalize(environmentMappingPosition - cameraPosition);
	vec3 refractiveVector = refract(viewDirection, normalize(environmentMappingNormal), refractionRatio);

	return vec4(vec3(texture(cubeMap, refractiveVector)), 1.0);
}

void main()
{
	// Use the reflection mapping vector function to set the frag color to render
	//fragColor = reflectionMapping();

	// Use the refraction mapping vector function to set the frag color to render
	fragColor = refractionMapping();
}