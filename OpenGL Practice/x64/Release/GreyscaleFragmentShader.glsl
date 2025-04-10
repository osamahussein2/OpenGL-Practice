#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D textureImage;

void main()
{
	fragColor = texture(textureImage, texCoords);

	// Take all the color components and averaging their results by dividing them using weighted RGB channels
	float average = fragColor.r * 0.2126 + fragColor.g * 0.7152 + fragColor.b * 0.0722;
	fragColor = vec4(average, average, average, 1.0);
}