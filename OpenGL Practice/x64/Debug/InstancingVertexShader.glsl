#version 330 core
layout (location = 0) in vec3 position;
//layout (location = 1) in vec3 color;
//layout (location = 2) in vec2 offset; // Instancing Part 1

layout (location = 2) in vec2 texCoordinates; // Instancing Part 2
layout (location = 3) in mat4 instanceMatrix; // Instancing Part 3

//out vec3 fragmentColor; // Instancing Part 1

//uniform vec2 offsets[100];

out vec2 texCoords; // Instancing Part 2

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
//uniform mat4 modelMatrix;

/* Instanced arrays are defined as a vertex attribute (allowing us to store much more data) that are updated per instance 
instead of per vertex */

/* With vertex attributes, at the start of each run of the vertex shader, the GPU will retrieve the next set of vertex 
attributes that belong to the current vertex. When defining a vertex attribute as an instanced array however, the vertex 
shader only updates the content of the vertex attribute per instance. This allows us to use the standard vertex attributes 
for data per vertex and use the instanced array for storing data that is unique per instance */

void main()
{
	/* Within the vertex shader we retrieve an offset vector for each instance by indexing the offsets array using 
	gl_InstanceID. If we now were to draw 100 quads with instanced drawing we’d get 100 quads located at different 
	positions */

	//vec2 offset = offsets[gl_InstanceID];

	// There's no gl_InstanceID and can directly use the offset attribute without first indexing into a large uniform array
	//gl_Position = vec4(position + offset, 0.0, 1.0);

	/* The result is that the first instances of the quads are drawn extremely small and the further we’re in the process of 
	drawing the instances, the closer gl_InstanceID gets to 100 and thus the more the quads regain their original size. It’s 
	perfectly legal to use instanced arrays together with gl_InstanceID like this */

	//vec2 pos = position + (gl_InstanceID / 100.0); // Instance Part 1

	//gl_Position = vec4(position + offset, 0.0, 1.0);
	//fragmentColor = color;

	// Instance Part 2
	//gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

	// Instance Part 3 (store an instanced array of transformation matrices instead of using model matrix)
	gl_Position = projectionMatrix * viewMatrix * instanceMatrix * vec4(position, 1.0);
	texCoords = texCoordinates;
}