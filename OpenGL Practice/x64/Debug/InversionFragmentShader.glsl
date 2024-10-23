#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D textureImage;

void main()
{
	fragColor = vec4(vec3(1.0 - texture(textureImage, texCoords)), 1.0);
}