#include "Blending.h"

Blending::Blending()
{
	transparentVAO = NULL;
	transparentVBO = NULL;

	transparentTexture = NULL;

	transparentData = NULL;

	width = NULL;
	height = NULL;
	nrChannels = NULL;

	blendingModelMatrix = mat4(0.0f);
}

Blending::~Blending()
{
	transparentVAO = NULL;
	transparentVBO = NULL;

	transparentTexture = NULL;

	transparentData = NULL;

	width = NULL;
	height = NULL;
	nrChannels = NULL;

	blendingModelMatrix = mat4(0.0f);
}

void Blending::SetBlending()
{
	float transparentVertices[] = {
		// position          // texture coordinate
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f, 1.0f,
		1.0f, -0.5f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.5f,  0.0f, 0.0f, 0.0f,
		1.0f, -0.5f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.5f, 0.0f, 1.0f, 0.0f
	};

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &transparentVAO);

	// Generate the buffer ID here
	glGenBuffers(1, &transparentVBO); // The & is a reference to the unsigned int of VBO and converts it to a GLuint pointer type

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), &transparentVertices, GL_STATIC_DRAW);

	// Bind the vertex array object using its ID
	glBindVertexArray(transparentVAO);

	// Set the position attribute's location to 0 like our vertex shader GLSL file
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0); // Position attribute location occurs at 0

	// Set the texture coordinates attribute's location to 1 like our vertex shader GLSL file
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(1); // Position attribute location occurs at 1

	// Generate the specular texture in OpenGL first before binding it
	glGenTextures(1, &transparentTexture);

	glBindTexture(GL_TEXTURE_2D, transparentTexture);

	// Use GL_CLAMP_TO_EDGE if we use alpha textures that shouldn't repeated
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load the grass image
	//transparentData = stbi_load("Textures/grass.png", &width, &height, &nrChannels, 0);

	// Load the transparent window image
	transparentData = stbi_load("Textures/blending_transparent_window.png", &width, &height, &nrChannels, 0);

	if (transparentData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, transparentData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	else
	{
		cout << "This texture has failed to load!" << endl;
	}

	// Free the image memory after generating the texture and its corresponding mipmaps
	stbi_image_free(transparentData);

	/* Bind the texture before calling the glDrawElements function and it will automatically assign the texture to
	the fragment shader's sampler */

	// Bind the specular map texture here

	glActiveTexture(GL_TEXTURE0); // Active the first texture unit
	glBindTexture(GL_TEXTURE_2D, transparentTexture);

	// Bind the vertex array object using its ID
	glBindVertexArray(transparentVAO);
}

void Blending::IncludeGrassBlending()
{
	vegetation.push_back(vec3(-1.5f, 0.0f, -0.48f));
	vegetation.push_back(vec3(1.5f, 0.0f, 0.51f));
	vegetation.push_back(vec3(0.0f, 0.0f, 0.7f));
	vegetation.push_back(vec3(-0.3f, 0.0f, -2.3f));
	vegetation.push_back(vec3(0.5f, 0.0f, -0.6f));

	for (unsigned int i = 0; i < vegetation.size(); i++)
	{
		blendingModelMatrix = mat4(1.0f);
		blendingModelMatrix = translate(blendingModelMatrix, vegetation[i]);

		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram::shaderProgram, "modelMatrix"), 1, GL_FALSE,
			value_ptr(blendingModelMatrix));

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

void Blending::IncludeTransparentWindowBlending()
{
	windows.push_back(vec3(-1.5f, 0.0f, -0.48f));
	windows.push_back(vec3(1.5f, 0.0f, 0.51f));
	windows.push_back(vec3(0.0f, 0.0f, 0.7f));
	windows.push_back(vec3(-0.3f, 0.0f, -2.3f));
	windows.push_back(vec3(0.5f, 0.0f, -0.6f));

	map<float, vec3> sorted;

	for (unsigned int i = 0; i < windows.size(); i++)
	{
		/* The result is a container map that stores each of the window positions based on their distance key value 
		from lowest to highest distance */
		float distance = length(Camera::cameraPosition - windows[i]);
		sorted[distance] = windows[i];
	}

	/* Take each of the map's values in reverse order (from farthest to nearest) and draw the corresponding windows
	in the correct order */
	for (map<float, vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++)
	{
		blendingModelMatrix = mat4(1.0f);
		blendingModelMatrix = translate(blendingModelMatrix, it->second);

		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram::shaderProgram, "modelMatrix"), 1, GL_FALSE,
			value_ptr(blendingModelMatrix));

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}
