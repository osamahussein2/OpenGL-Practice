#version 330 core
out vec4 fragColor;

//in vec3 fragmentColor; // Instancing Part 1

in vec2 texCoords; // Instancing Part 2

uniform sampler2D textureImage; // Instancing Part 2

void main()
{
	//fragColor = vec4(fragmentColor, 1.0); // Instancing Part 1

	fragColor = texture(textureImage, texCoords); // Instancing Part 2
}