#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D textureImage;

uniform float near;
uniform float far;

float LinearDepthValue(float depth)
{
	// Transform the depth value to Normal Device Coordinates (range from -1 and 1)
	float NDC = depth * 2.0 - 1.0;

	/* Reverse the non-linear as done in the projection matrix and apply its inversed equation to the resulting depth 
	value */
	return (2.0 * near * far) / (far + near - NDC * (far - near));
}

void main()
{
	//fragColor = texture(textureImage, texCoords);

	// OpenGL's built in gl_FragCoord vector contains the depth value of that particular fragment
	//fragColor = vec4(vec3(gl_FragCoord.z), 1.0);

	//fragColor = vec4(vec3(LinearDepthValue(gl_FragCoord.z) / far), 1.0);

	// Create a local vec4 for the texture color to use it inside of the conditional statement
	vec4 textureColor = texture(textureImage, texCoords);

	// If the alpha channel of the fragment is less than 0.1, discard it as if it's never been processed
	if (textureColor.a < 0.1)
	{
		discard;
	}

	// Use the newly created texture color vector and make it equal to the fragment color
	fragColor = textureColor;
}