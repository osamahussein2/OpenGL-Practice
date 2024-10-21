#include "FaceCulling.h"

FaceCulling::FaceCulling()
{
	faceCullingVAO = NULL;
	faceCullingVBO = NULL;

	faceCullingTexture = NULL;

	faceCullingData = NULL;

	width = NULL;
	height = NULL;
	nrChannels = NULL;
}

FaceCulling::~FaceCulling()
{
	glDeleteVertexArrays(1, &faceCullingVAO);
	glDeleteBuffers(1, &faceCullingVBO);

	faceCullingTexture = NULL;

	faceCullingData = NULL;

	width = NULL;
	height = NULL;
	nrChannels = NULL;
}

void FaceCulling::SetFaceCullingVertices()
{
	// Enable face culling
	glEnable(GL_CULL_FACE);

	/* glCullFace function only has 3 options:
		
		GL_BACK - culls only the back faces
		GL_FRONT - culls only the front faces
		GL_FRONT_AND_BACK - culls both the front and back faces

	*/

	// Cull the back faces only
	glCullFace(GL_BACK);
	
	// Tell OpenGL we prefer clockwise faces as the front faces instead of counter-clockwise faces via glFrontFace

	/* The default value is GL_CCW, which stands for counter - clockwise ordering with the other option being GL_CW which
	stands for clockwise ordering */
	glFrontFace(GL_CW);

	cubeVertices =
	{
		// back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right    
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right              
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left    

		// front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right        
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left

		// left face
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left       
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
	
		// right face
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right      
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		
		// bottom face
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right

		// top face
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f  // top-left
	};

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &faceCullingVAO);

	// Generate the buffer ID here
	glGenBuffers(1, &faceCullingVBO); // The & is a reference to the unsigned int of VBO and converts it to a GLuint pointer type

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, faceCullingVBO);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

	// Bind the vertex array object using its ID
	glBindVertexArray(faceCullingVAO);

	// Set the position attribute's location to 0 like our vertex shader GLSL file
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0); // Position attribute location occurs at 0

	// Set the texture coordinates attribute's location to 1 like our vertex shader GLSL file
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(1); // Position attribute location occurs at 1

	// Generate the specular texture in OpenGL first before binding it
	glGenTextures(1, &faceCullingTexture);

	glBindTexture(GL_TEXTURE_2D, faceCullingTexture);

	// Use GL_CLAMP_TO_EDGE if we use alpha textures that shouldn't repeated
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	faceCullingData = stbi_load("Textures/checkerboard.jpg", &width, &height, &nrChannels, 0);

	if (faceCullingData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, faceCullingData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	else
	{
		cout << "This texture has failed to load!" << endl;
	}

	// Free the image memory after generating the texture and its corresponding mipmaps
	stbi_image_free(faceCullingData);

	/* Bind the texture before calling the glDrawElements function and it will automatically assign the texture to
	the fragment shader's sampler */

	// Bind the specular map texture here

	glActiveTexture(GL_TEXTURE0); // Active the first texture unit
	glBindTexture(GL_TEXTURE_2D, faceCullingTexture);
}