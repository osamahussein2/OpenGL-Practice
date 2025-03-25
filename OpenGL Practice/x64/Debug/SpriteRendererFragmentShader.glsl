#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D image;
uniform vec3 spriteColor;

void main()
{
	// Set the color of the sprite by multiplying sprite color with the texture
	fragColor = vec4(spriteColor, 1.0) * texture(image, texCoords);
}