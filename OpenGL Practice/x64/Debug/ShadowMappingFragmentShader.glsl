#version 330 core

out vec4 fragColor;

in VS_OUT 
{
	vec3 fragPosition;
	vec3 normal;
	vec2 texCoords;
	vec4 fragPositionLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPosition;
uniform vec3 viewPosition;

// Shadow Mapping Part 2
float ShadowCalculation(vec4 fragPositionLightSpace_)
{
	/* The first thing to do to check whether a fragment is in shadow, is transform the light-space fragment position in 
	clip-space to normalized device coordinates */

	
	// Do perspective division
	vec3 projCoords = fragPositionLightSpace_.xyz / fragPositionLightSpace_.w;

	/* Because the depth from the depth map is in the range from 0 to 1 and we also want to use projCoords to sample from the 
	depth map, we transform the NDC coordinates to the range from 0 to 1 */

	projCoords = projCoords * 0.5 + 0.5;

	/* With these projected coordinates we can sample the depth map as the resulting [0,1] coordinates from projCoords 
	directly correspond to the transformed NDC coordinates from the first render pass. This gives us the closest depth from 
	the light’s point of view */

	float closestDepth = texture(shadowMap, projCoords.xy).r;

	/* To get the current depth at this fragment we simply retrieve the projected vector’s z coordinate which equals the 
	depth of this fragment from the light’s perspective */

	float currentDepth = projCoords.z;

	// calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.normal);
    vec3 lightDir = normalize(lightPosition - fs_in.fragPosition);

	/* With the bias applied, all the samples get a depth smaller than the surface’s depth and thus the entire surface is 
	correctly lit without any shadows (Shadow Mapping Part 3) */

	/* This way, surfaces like the floor that are almost perpendicular to the light source get a small bias, while 
	surfaces like the cube’s side-faces get a much larger bias */

	// Set bias to be based on the surface angle towards the light (using the dot product)
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

	// Shadow Mapping Part 3
	/* Because the depth map has a fixed resolution, the depth frequently usually spans more than one fragment per texel. 
	As a result, multiple fragments sample the same depth value from the depth map and come to the same shadow conclusions, 
	which produces these jagged blocky edges */

	/* A (partial) solution to these jagged edges is called PCF, or percentage-closer filtering, which is a term that hosts 
	many different filtering functions that produce softer shadows, making them appear less blocky or hard. The idea is to 
	sample more than once from the depth map, each time with slightly different texture coordinates */

	float shadow = 0;

	// textureSize returns a vec2 of the width and height of the given sampler texture at mipmap level 0

	/* 1 divided over this returns the size of a single texel that we use to offset the texture coordinates, making sure 
	each new sample samples a different depth value */

	// By using more samples and/or varying the texelSize variable you can increase the quality of the soft shadows
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			/* Sample 9 values around the projected coordinate’s x and y value, test for shadow occlusion, and finally 
			average the results by the total number of samples taken */

			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;

			/* The actual comparison is then simply a check whether currentDepth is higher than closest Depth and if so, 
			the fragment is in shadow */

			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;

	// Force the shadow value to 0.0 whenever the projected vector’s z coordinate is larger than 1.0
	if(projCoords.z > 1.0)
	{
		shadow = 0.0;
	}

	return shadow;
}

void main()
{
	vec3 color = texture(diffuseTexture, fs_in.texCoords).rgb;
	vec3 normal = normalize(fs_in.normal);
	vec3 lightColor = vec3(0.3);

	// ambient
	vec3 ambient = 0.3 * color;

	// diffuse
	vec3 lightDir = normalize(lightPosition - fs_in.fragPosition);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * lightColor;

	// specular
	vec3 viewDir = normalize(viewPosition - fs_in.fragPosition);
	float spec = 0.0;
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor;

	// calculate shadow
	float shadow = ShadowCalculation(fs_in.fragPositionLightSpace);

	/* Multiply the diffuse and specular contributions by the inverse of the shadow component e.g. how much the fragment 
	is not in shadow */

	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

	fragColor = vec4(lighting, 1.0);
}