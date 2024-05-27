#version 330 core // Version of GLSL with OpenGL's version
layout (location = 0) in vec3 position;

void main()
{
	// gl_Position is a 4D vector inside of OpenGL 
	gl_Position = vec4(position.x, position.y, position.z, 1.0);
}