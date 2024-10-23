#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D textureImage;

void main()
{
	// Create a local vec4 for the texture color
	vec4 textureColor = texture(textureImage, texCoords);

	// Use the newly created texture color vector and make it equal to the fragment color
	fragColor = textureColor;
}