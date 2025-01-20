#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D depthMap;

uniform float nearPlane;
uniform float farPlane;

// Required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main()
{
	float depthValue = texture(depthMap, texCoords).r;

	/* There is a difference between rendering the depth map with an orthographic or a projection matrix. An orthographic 
	projection matrix does not deform the scene with perspective so all view/light rays are parallel. This makes it a great 
	projection matrix for directional lights. A perspective projection matrix however does deform all vertices based on 
	perspective which gives different results */

	/* Perspective projections make most sense for light sources that have actual locations, unlike directional lights. 
	Perspective projections are most often used with spotlights and point lights, while orthographic projections are used 
	for directional lights */

	/* Another subtle difference with using a perspective projection matrix is that visualizing the depth buffer will often 
	give an almost completely white result. This happens because with perspective projection the depth is transformed to 
	non-linear depth values with most of its noticeable range close to the near plane */

	//fragColor = vec4(vec3(LinearizeDepth(depthValue) / farPlane), 1.0); // perspective

	fragColor = vec4(vec3(depthValue), 1.0); // orthographic
}