#version 330 core
out vec4 fragColor;

in vec3 vertexColor; // Receives the vertex color we set inside the vertex shader and we can use it here
in vec3 vertexPosition;
in vec2 textureCoord;

uniform sampler2D theFirstTexture;
uniform sampler2D theSecondTexture;

void main()
{
	//fragColor = texture(theFirstTexture, textureCoord) * vec4(vertexColor, 1.0);

	fragColor = mix(texture(theFirstTexture, textureCoord),  texture(theSecondTexture, textureCoord), 0.2);
}