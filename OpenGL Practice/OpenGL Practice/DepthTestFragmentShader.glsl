#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D textureImage;

void main()
{
	fragColor = texture(textureImage, texCoords);
}