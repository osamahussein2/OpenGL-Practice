#include "Skybox.h"

/* A cubemap is a texture that contains 6 individual 2D textures that each form one side of a cube : a textured cube.
Cube maps have the useful property that they can be indexed/sampled using a direction vector. */

/* OpenGL has 6 special texture targets for targeting the face of a cubemap:

	1. GL_TEXTURE_CUBE_MAP_POSITIVE_X - right orientation
	2. GL_TEXTURE_CUBE_MAP_NEGATIVE_X - left orientation
	3. GL_TEXTURE_CUBE_MAP_POSITIVE_Y - top orientation
	4. GL_TEXTURE_CUBE_MAP_NEGATIVE_Y - bottom orientation
	5. GL_TEXTURE_CUBE_MAP_POSITIVE_Z - back orientation
	6. GL_TEXTURE_CUBE_MAP_NEGATIVE_Z - front orientation

*/

Skybox::Skybox()
{
	skyboxVAO = NULL;
	skyboxVBO = NULL;

	skyboxTexture = NULL;
	skyboxData = NULL;
	
	skyboxVertices = { NULL };

	width = NULL;
	height = NULL;
	nrChannels = NULL;

	modelMatrix = mat4(0.0f);
	projectionMatrix = mat4(0.0f);
	viewMatrix = mat4(0.0f);
}

Skybox::~Skybox()
{
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	skyboxTexture = NULL;
	skyboxData = NULL;

	skyboxVertices = { NULL };

	width = NULL;
	height = NULL;
	nrChannels = NULL;

	modelMatrix = mat4(0.0f);
	projectionMatrix = mat4(0.0f);
	viewMatrix = mat4(0.0f);
}

void Skybox::SetSkyboxObject()
{
	skyboxVertices =
	{
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);

	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	// Position attribute location
	// I'm using sizeof vec3 since we have vector3 for position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	glEnableVertexAttribArray(0);

	// Push back the skybox textures vector here
	skyboxTextures.push_back("Textures/Skybox/SkyboxRight.jpg");
	skyboxTextures.push_back("Textures/Skybox/SkyboxLeft.jpg");
	skyboxTextures.push_back("Textures/Skybox/SkyboxTop.jpg");
	skyboxTextures.push_back("Textures/Skybox/SkyboxBottom.jpg");
	skyboxTextures.push_back("Textures/Skybox/SkyboxFront.jpg");
	skyboxTextures.push_back("Textures/Skybox/SkyboxBack.jpg");
}

void Skybox::SetSkyboxTexture()
{
	// Generate the texture in OpenGL first before binding it
	glGenTextures(1, &skyboxTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture); // Bind to a cube map texture

	for (unsigned int i = 0; i < skyboxTextures.size(); i++)
	{
		skyboxData = stbi_load(skyboxTextures[i].c_str(), &width, &height, &nrChannels, 0);

		if (skyboxData)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB,
				GL_UNSIGNED_BYTE, skyboxData);
		}

		else
		{
			cout << "This texture has failed to load!" << std::endl;
		}

		// Free the image memory after generating the texture and its corresponding mipmaps
		stbi_image_free(skyboxData);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Set the wrapping method for a skybox to GL_CLAMP_TO_EDGE since texture coordinates that are exactly between 2
	faces may not hit an exact face (due to some hardware limitations) so by using GL_CLAMP_TO_EDGE, OpenGL always
	returns their edge values whenever we sample between faces. */
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // Added for 3D textures
}

void Skybox::UseShaderProgramForSkybox(float aspect_ratio, float near_plane, float far_plane)
{
	/*glUseProgram(ShaderProgram::shaderProgram);

	glUniform1i(glGetUniformLocation(ShaderProgram::shaderProgram, "cubeMap"), 0);

	modelMatrix = mat4(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(ShaderProgram::shaderProgram, "skyboxModelMatrix"), 1, GL_FALSE,
		glm::value_ptr(modelMatrix));

	projectionMatrix = perspective(radians(Camera::fieldOfView), aspect_ratio, near_plane, far_plane);

	glUniformMatrix4fv(glGetUniformLocation(ShaderProgram::shaderProgram, "skyboxProjectionMatrix"), 1, GL_FALSE,
		glm::value_ptr(projectionMatrix));

	viewMatrix = mat4(mat3(lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp)));

	glUniformMatrix4fv(glGetUniformLocation(ShaderProgram::shaderProgram, "skyboxViewMatrix"), 1, GL_FALSE,
		glm::value_ptr(viewMatrix));*/

	// Bind the vertex array object using its ID
	glBindVertexArray(skyboxVAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void Skybox::SetCubeObject()
{
	cubeVertices = 
	{
		// positions          // normals
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &cubeVAO);

	// Generate the buffer ID here
	glGenBuffers(1, &cubeVBO);

	// Bind the vertex array object using its ID
	glBindVertexArray(cubeVAO);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

	// Create a variable to store the size of the position and normal vectors combined
	unsigned int vectorsSize = sizeof(vec3) + sizeof(vec3);

	// Set the position attribute's location to 0 like our vertex shader GLSL file
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vectorsSize, (void*)0);

	glEnableVertexAttribArray(0); // Position attribute location occurs at 0

	// Set the normals attribute's location to 1 like our vertex shader GLSL file
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vectorsSize, (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(1); // Normal attribute location occurs at 1
}

void Skybox::SetCubeTexture()
{
	// Generate the texture in OpenGL first before binding it
	glGenTextures(1, &cubeTexture);

	// Then, we need to bind the textures to configure the currently bound texture on subsequent texture commands
	glBindTexture(GL_TEXTURE_2D, cubeTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	cubeData = stbi_load("Textures/container.jpg", &width, &height, &nrChannels, 0);

	// After the texture ahs been binded, we can generate textures using the previously loaded image data
	// Textures are generated with glTexImage2D
	if (cubeData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, cubeData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	else
	{
		std::cout << "This texture has failed to load!" << std::endl;
	}

	// Free the image memory after generating the texture and its corresponding mipmaps
	stbi_image_free(cubeData);
}

void Skybox::UseShaderProgramForCube(float aspect_ratio, float near_plane, float far_plane)
{
	/*glUseProgram(ShaderProgram::shaderProgram);

	glUniform1i(glGetUniformLocation(ShaderProgram::shaderProgram, "textureImage"), 0);

	glUniform3fv(glGetUniformLocation(ShaderProgram::shaderProgram, "cameraPosition"), 1, 
		value_ptr(Camera::cameraPosition));

	// Set the refraction ratio uniform here (only for the refraction environment mapping)
	glUniform1f(glGetUniformLocation(ShaderProgram::shaderProgram, "refractionRatio"), (1.00f / 1.52f));

	projectionMatrix = glm::perspective(glm::radians(Camera::fieldOfView), aspect_ratio, near_plane, far_plane);

	glUniformMatrix4fv(glGetUniformLocation(ShaderProgram::shaderProgram, "projectionMatrix"), 1, GL_FALSE, 
		glm::value_ptr(projectionMatrix));

	viewMatrix = Camera::CameraLookAt();

	glUniformMatrix4fv(glGetUniformLocation(ShaderProgram::shaderProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	modelMatrix = glm::mat4(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(ShaderProgram::shaderProgram, "modelMatrix"), 1,
		GL_FALSE, glm::value_ptr(modelMatrix));*/

	glBindVertexArray(cubeVAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}
