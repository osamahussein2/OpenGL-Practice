#version 330 core
out vec4 fragColor;

in vec3 vertexColor; // Receives the vertex color we set inside the vertex shader and we can use it here
in vec3 vertexPosition;

void main()
{
	fragColor = vec4(vertexPosition, 1.0f);
}