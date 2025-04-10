#version 330 core

layout (location = 0) in vec4 vertex; // vec2s for position and texture coordinates

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform vec2 offset;
uniform vec4 color;

void main()
{
	float scale = 10.0f;

	// Take the standard position and texture attributes per particle and also accept an offset and a color uniform for changing the outcome per particle

	TexCoords = vertex.zw;
	ParticleColor = color;

	gl_Position = projection * vec4((vertex.xy * scale) + offset, 0.0, 1.0);
}