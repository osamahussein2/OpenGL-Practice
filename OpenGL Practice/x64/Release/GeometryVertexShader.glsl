#version 330 core

layout (location = 0) in vec3 position;
//layout (location = 1) in vec3 vertexColor;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 TexCoordinates;

out VS_OUT {
	//vec3 color;
	//vec2 texCoords;
	vec3 normal;
} vs_out;

//uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main()
{
	// Geometry Shader Part 1
	//gl_Position = vec4(position.x, position.y, 0.0, 1.0);
	//vs_out.color = vertexColor;

	// Geometry Shader Part 2
	//vs_out.texCoords = TexCoordinates;
	//gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0); 

	// Geometry Shader Part 3

	/* To accommodate for scaling and rotations (due to the view and model matrix) we’ll transform the normals with a normal 
	matrix. The geometry shader receives its position vectors as view-space coordinates so we should also transform the 
	normal vectors to the same space */

	gl_Position = viewMatrix * modelMatrix * vec4(position, 1.0);
	mat3 normalMatrix = mat3(transpose(inverse(viewMatrix * modelMatrix)));
	vs_out.normal = normalize(vec3(vec4(normalMatrix * normals, 0.0)));

}
