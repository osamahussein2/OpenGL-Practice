#include "FrameBuffer.h"

FrameBuffer::FrameBuffer()
{
	cubeVAO = NULL;
	cubeVBO = NULL;

	planeVAO = NULL;
	planeVBO = NULL;

	quadVAO = NULL;
	quadVBO = NULL;

	frameBufferFBO = NULL;
	frameBufferRBO = NULL;

	frameBufferTexture = NULL;

	frameBufferData = NULL;

	width = NULL;
	height = NULL;
	nrChannels = NULL;

	cubeVertices = { NULL };
}

FrameBuffer::~FrameBuffer()
{
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);

	glDeleteFramebuffers(1, &frameBufferFBO);
	glDeleteRenderbuffers(1, &frameBufferRBO);

	frameBufferTexture = NULL;

	frameBufferData = NULL;

	width = NULL;
	height = NULL;
	nrChannels = NULL;
}

void FrameBuffer::InitializeCubeVertices()
{
	cubeVertices =
	{
		// back face
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, // bottom-right
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f, // top-right
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left

		// front face
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, // bottom-right
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f, // top-right
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f, // top-right
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, // top-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left

		// left face
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // top-right
		-0.5f, 0.5f, -0.5f, 1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-right
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // top-right

		// right face
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // top-left
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-right
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-right
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // top-left
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left

		// bottom face
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f, // top-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, // bottom-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right

		// top face
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // bottom-right
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f, // top-right
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // bottom-right
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f // bottom-left
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

	// Set the position attribute's location to 0 like our vertex shader GLSL file
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0); // Position attribute location occurs at 0

	// Set the texture coordinates attribute's location to 1 like our vertex shader GLSL file
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(1); // Position attribute location occurs at 1
}

void FrameBuffer::InitializePlaneVertices()
{
	planeVertices =
	{
		// position           // texture coord
		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,  2.0f, 2.0f
	};

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &planeVAO);

	// Generate the buffer ID here
	glGenBuffers(1, &planeVBO);

	// Bind the vertex array object using its ID
	glBindVertexArray(planeVAO);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

	// Set the position attribute's location to 0 like our vertex shader GLSL file
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0); // Position attribute location occurs at 0

	// Set the texture coordinates attribute's location to 1 like our vertex shader GLSL file
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(1); // Position attribute location occurs at 1
}

void FrameBuffer::InitializeQuadVertices()
{
	quadVertices =
	{
		// positions // texCoords
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		1.0f, 1.0f,  0.0f, 1.0f, 1.0f
	};

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &quadVAO);

	// Generate the buffer ID here
	glGenBuffers(1, &quadVBO);

	// Bind the vertex array object using its ID
	glBindVertexArray(quadVAO);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	// Set the position attribute's location to 0 like our vertex shader GLSL file
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0); // Position attribute location occurs at 0

	// Set the texture coordinates attribute's location to 1 like our vertex shader GLSL file
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(1); // Position attribute location occurs at 1
}

void FrameBuffer::AddFrameBuffer()
{

	// Generate the frame buffer ID
	glGenFramebuffers(1, &frameBufferFBO);

	/* By binding to the GL_FRAMEBUFFER target all the next read and write framebuffer operations will affect the
	currently bound framebuffer */

	/* It's possible to only bind a framebuffer to a read or write target by bidning to GL_READ_FRAMEBUFFER or
	GL_DRAW_FRAMEBUFFER, respectively */

	/* The GL_READ_FRAMEBUFFER is used for all read operations like glReadPixels and GL_DRAW_FRAMEBUFFER is used as
	the destination for rendering, clearing and other write operations */
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferFBO);

	// Generate the specular texture in OpenGL first before binding it
	glGenTextures(1, &frameBufferTexture);

	glBindTexture(GL_TEXTURE_2D, frameBufferTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* glFramebufferTexture2D passes in 5 parameters:

		target - the framebuffer type we're targeting (draw, read, or both)
		attachment - the type of attachment we're going to attach (e.g. color)
		textarget - the type of texture to be attached
		texture - the actual texture to attach
		level - mipmap level (keep it at 0 for now)

	*/

	// Attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);

	/* Next to the color attachments we can also attach a depth and a stencil texture to the framebuffer object. To
	attach a depth attachment we specify the attachment type as GL_DEPTH_ATTACHMENT. However though, the texture's
	format and internalformat type of attachment is GL_DEPTH_COMPONENT to reflect the depth buffer's storage format.
	To attach a stencil buffer, use GL_STENCIL_ATTACHMENT as the second argument and specify the texture's formats as
	GL_STENCIL_INDEX. */

	/* It's also possible to attach both a depth buffer and stencil buffer as a single texture. Each 32 bit value of
	the texture then contains 24 bits of depth information and 8 bits of stencil information. To attach a depth and
	stencil buffer as one texture, we use the GL_DEPTH_STENCIL_ATTACHMENT type and configure the texture's formats to
	contain combined depth and stencil values.

	An example of attaching a depth and stencil buffer as one texture to the frame buffer:

	glTexImage2D(GL_TEXTURE2D, 0, GL_DEPTH24_STENCIL8, 800, 600, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);

	*/

	// Generate the render buffer object ID
	glGenRenderbuffers(1, &frameBufferRBO);

	// Bind the render buffer object to the GL_RENDERBUFFER target
	glBindRenderbuffer(GL_RENDERBUFFER, frameBufferRBO);

	// Creating a depth and stencil renderbuffer object is done by calling the glRenderbufferStorage function
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);

	/* Creating a renderbuffer object is similar to texture objects, the difference being that this object is specifically
	designed to be used as a framebuffer attachment, instead of a general purpose data buffer like a texture */
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frameBufferRBO);

	/* For a framebuffer to be complete, there are requirements that need to be met:

		At least 1 buffer has to be attached (color, depth or stencil)
		There should be at least 1 color attachment
		All attachments should be complete as well (reserved memory)
		Each buffer should have the same number of samples

	*/

	/* After these requirements are met, we can check if we successfully completed the framebuffer by calling
		glCheckFramebufferStatus with GL_FRAMEBUFFER. It then checks the currently bound framebuffer and returns an
		enum of values found in the specification. If it returns GL_FRAMEBUFFER_COMPLETE we're good to go. */
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::RenderScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferFBO);
	glClearColor(0.05f, 0.05f, 0.05f, 0.05f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
}

void FrameBuffer::InitializeCubeTextures()
{
	// Generate the texture in OpenGL first before binding it
	glGenTextures(1, &cubeTexture);

	// Then, we need to bind the textures to configure the currently bound texture on subsequent texture commands
	glBindTexture(GL_TEXTURE_2D, cubeTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	frameBufferData = stbi_load("Textures/container.jpg", &width, &height, &nrChannels, 0);

	// After the texture ahs been binded, we can generate textures using the previously loaded image data
	// Textures are generated with glTexImage2D
	if (frameBufferData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, frameBufferData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	else
	{
		std::cout << "This texture has failed to load!" << std::endl;
	}

	// Free the image memory after generating the texture and its corresponding mipmaps
	stbi_image_free(frameBufferData);

	// Bind the vertex array object using its ID
	glBindVertexArray(cubeVAO);

	// Bind the diffuse map texture here
	glActiveTexture(GL_TEXTURE0); // Active the first texture unit first before binding it
	glBindTexture(GL_TEXTURE_2D, cubeTexture);
}

void FrameBuffer::InitializePlaneTextures()
{
	// Generate the specular texture in OpenGL first before binding it
	glGenTextures(1, &planeTexture);

	// Then, we need to bind the textures to configure the currently bound texture on subsequent texture commands
	glBindTexture(GL_TEXTURE_2D, planeTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	frameBufferData = stbi_load("Textures/metal.png", &width, &height, &nrChannels, 0);

	if (frameBufferData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, frameBufferData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	else
	{
		std::cout << "This texture has failed to load!" << std::endl;
	}

	// Free the image memory after generating the texture and its corresponding mipmaps
	stbi_image_free(frameBufferData);

	// Bind the vertex array object using its ID
	glBindVertexArray(planeVAO);

	/* Bind the texture before calling the glDrawElements function and it will automatically assign the texture to
	the fragment shader's sampler */
	// Bind the specular map texture here
	glBindTexture(GL_TEXTURE_2D, planeTexture);
}

void FrameBuffer::InitializeQuadTextures()
{
	glBindVertexArray(quadVAO);
	glBindTexture(GL_TEXTURE_2D, frameBufferTexture); // use the color attachment texture as the texture of the quad plane
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void FrameBuffer::BindToDefaultFrameBuffer()
{
	// Bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Disable depth test so screen-space quad isn't discarded due to depth test
	glDisable(GL_DEPTH_TEST);

	// Clear only the color buffer
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}
