#version 330 core

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 brightColor;

in SHADER_VARIABLES {
    vec3 fragPosition;
    vec3 normal;
    vec2 texCoords;
} shader_fs;

uniform vec3 lightColor;

void main()
{
    // Use what is currently stored in frag color to determine if its brightness exceeds a certain threshold
    fragColor = vec4(lightColor, 1.0);

    /* Calculate the brightness of a fragment by properly transforming it to grayscale first (by taking the dot product of 
    both vectors we effectively multiply each individual component of both vectors and add the results together) */

    float brightness = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));

    // If the brightness exceeds a certain threshold, output the color to the second color buffer using brightColor
    if(brightness > 1.0)
        brightColor = vec4(fragColor.rgb, 1.0);
	else
		brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}