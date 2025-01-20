#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoordinates;

out VS_OUT {
	vec3 fragPosition;
	vec3 normal;
	vec2 texCoords;

	/* This takes the same lightSpaceMatrix (used to transform vertices to light space in the depth map stage) and transform 
	the world-space vertex position to light space for use in the fragment shader */

	vec4 fragPositionLightSpace;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;
void main()
{
	vs_out.fragPosition = vec3(modelMatrix * vec4(position, 1.0));
    vs_out.normal = transpose(inverse(mat3(modelMatrix))) * normals;
    vs_out.texCoords = texCoordinates;
    vs_out.fragPositionLightSpace = lightSpaceMatrix * vec4(vs_out.fragPosition, 1.0);

    gl_Position = projection * view * modelMatrix * vec4(position, 1.0);
}
