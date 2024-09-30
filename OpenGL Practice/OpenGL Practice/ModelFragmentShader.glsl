#version 330 core

out vec4 fragColor;

in vec2 textureCoords;

uniform sampler2D TextureDiffuse1;

void main()
{
	fragColor = texture(TextureDiffuse1, textureCoords);
}