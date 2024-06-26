#version 330 core // Version of GLSL with OpenGL's version
layout (location = 0) in vec3 position; // vertex attribute position is equal to 0
layout (location = 1) in vec3 color; // color attribute position is equal to 1

out vec3 vertexColor; // The sending vertex shader to use inside the fragment shader
out vec3 vertexPosition;

uniform vec3 movingPosition;

void main()
{
	// gl_Position is a 4D vector inside of OpenGL 
	gl_Position = vec4(-position + movingPosition, 1.0);

	// Input the color from the vertex data
	vertexPosition = position;
}