#version 330 core

out vec4 fragColor;

uniform float redValue;
uniform float greenValue;
uniform float blueValue;
uniform float alphaValue;

void main()
{
	// Hardcode a border color
	fragColor = vec4(redValue, greenValue, blueValue, alphaValue);
}