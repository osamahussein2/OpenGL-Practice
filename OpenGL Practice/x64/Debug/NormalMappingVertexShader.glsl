#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 biTangent;

out SHADER_OUT
{
	vec3 fragPos;
	vec2 textureCoords;
	vec3 tangentLightPosition;
	vec3 tangentViewPosition;
	vec3 tangentFragPosition;

} normalMapping_vs;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform vec3 lightPosition;
uniform vec3 viewPosition;

void main()
{
	normalMapping_vs.fragPos = vec3(modelMatrix * vec4(position, 1.0));
	normalMapping_vs.textureCoords = texCoords;

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

	/* Transform all the TBN vectors to the coordinate system we’d like to work in, which in this case is world-space as we 
	multiply them with the model matrix. Then we create the actual TBN matrix by directly supplying mat3’s constructor with 
	the relevant vectors */

	vec3 T = normalize(normalMatrix * tangent);
	vec3 N = normalize(normalMatrix * normals);

	T = normalize(T - dot(T, N) * N);

	vec3 B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));

	normalMapping_vs.tangentLightPosition = TBN * lightPosition;
	normalMapping_vs.tangentViewPosition = TBN * viewPosition;
	normalMapping_vs.tangentFragPosition = TBN * normalMapping_vs.fragPos;

	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}