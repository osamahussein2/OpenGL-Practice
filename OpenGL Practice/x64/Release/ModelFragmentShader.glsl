#version 330 core

out vec4 fragColor;

in vec2 textureCoords;

uniform sampler2D TextureDiffuse;

void main()
{
	fragColor = texture(TextureDiffuse, textureCoords);
}