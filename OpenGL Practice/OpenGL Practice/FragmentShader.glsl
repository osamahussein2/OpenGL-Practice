#version 330 core
out vec4 fragColor;

in vec3 vertexColor; // Receives the vertex color we set inside the vertex shader and we can use it here
in vec3 vertexPosition;
in vec2 textureCoord;
vec2 otherDirection; // Add a vector that makes the smiley face look in the other direction

uniform sampler2D theFirstTexture;
uniform sampler2D theSecondTexture;

void main()
{
	//fragColor = texture(theFirstTexture, textureCoord) * vec4(vertexColor, 1.0);

	// Make the x a negative 1 to look in the opposite direction horizontally but keep the y value to 1
	// The y value should be 1 so that the face is visible on the container texture
	otherDirection = vec2(-1.0, 1.0);

	fragColor = mix(texture(theFirstTexture, textureCoord),  texture(theSecondTexture, textureCoord * 
	otherDirection), 0.2);
}