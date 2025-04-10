#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

out vec2 TexCoords;

uniform mat4 projection;

void main()
{
	/* Combine both the position and texture coordinate data into one vec4. The vertex shader multiplies the coordinates with
	a projection matrix and forwards the texture coordinates to the fragment shader */

	gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
	TexCoords = vertex.zw;
}