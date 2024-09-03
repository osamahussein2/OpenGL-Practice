#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 resultingLight;

// Send the fragment position and the normal to the fragment shader so that we can set them there
out vec3 FragPosition;
out vec3 Normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform vec3 objectColor;
uniform vec3 lightColor;

uniform vec3 lightPosition;
uniform vec3 viewPosition;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

	// Multiply the fragment position using the model matrix to transform it to world space coordinates
	FragPosition = vec3(modelMatrix * vec4(position, 1.0));

	// Generate the normal matrix by using the transpose and inverse functions (only if the vec3 is non-uniform scale)
	// The matrix has to be casted as a mat3 to make sure it doesn't have translation properties anymore
	Normal =  mat3(transpose(inverse(modelMatrix))) * normal;

	/* The lighting looks a bit weird when I implemented the Gouraud shading because the lighting shader applies
	to every single vertice that is being instantiated into the vertex shader and the lighting won't be as realistic
	compared to the fragment shader where the lighting will look a lot more realistic. */

	// Ambient lighting
	float ambientLightBrightness = 0.1;
	vec3 ambientLight = ambientLightBrightness * lightColor;

	// Diffuse lighting
	vec3 normalizeNormals = normalize(Normal);
	vec3 lightDirection = normalize(lightPosition - FragPosition);

	vec3 diffuseLight = max(dot(normalizeNormals, lightDirection), 0.0) * lightColor;

	float specularBrightness = 0.5;

	vec3 viewDirection = normalize(viewPosition - FragPosition);
	vec3 reflectionDirection = reflect(-lightDirection, normalizeNormals);

	float specularValue = pow(max(dot(viewDirection, reflectionDirection), 0.0), 32);
	vec3 specularLight = specularBrightness * specularValue * lightColor;

	resultingLight = (ambientLight + diffuseLight + specularLight) * objectColor;
}