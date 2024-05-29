#version 330 core // Version of GLSL with OpenGL's version
layout (location = 0) in vec3 position; // vertex attribute position is equal to 0

out vec4 vertexColor; // The sending vertex shader

void main()
{
	// gl_Position is a 4D vector inside of OpenGL 
	gl_Position = vec4(position.x, position.y, position.z, 1.0);

	// This vertex color will be dark-red
	vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
}