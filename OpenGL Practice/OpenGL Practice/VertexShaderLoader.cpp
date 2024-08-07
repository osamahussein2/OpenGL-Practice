// The preprocessor modifies the header file to contain only the relevant definition source code
// Turns this header file below into a .cpp file
#define STB_IMAGE_IMPLEMENTATION

#include "VertexShaderLoader.h"

VertexShaderLoader::VertexShaderLoader(const char* vertexShaderPath_)
{
	// Make sure the vertex shader file ifstream will throw an exception error
	vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// Open the vertex shader file
		vertexShaderFile.open(vertexShaderPath_);

		// Read the vertex shader's file buffer contents into a stream of the vertex shader file
		vertexShaderStream << vertexShaderFile.rdbuf();

		// Close the vertex shader's file handlers
		vertexShaderFile.close();

		// Convert the vertex shader stream into a vertex shader string
		vertexShaderCode = vertexShaderStream.str();
	}

	catch (std::ifstream::failure error)
	{
		std::cout << "ERROR::SHADER::VERTEX::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}

	vShaderCode = vertexShaderCode.c_str();

	VAO = NULL;
	VBO = NULL;

	EBO = NULL;

	vertexShader = NULL;

	texture1 = NULL;
	texture2 = NULL;

	//shaderProg = ShaderProgram();
}

void VertexShaderLoader::InitializeVertexShaderLoader()
{
	// Create a shader object ID to use it when we create the shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create a vertex shader using OpenGL's version of it

	// Attach the shader source code (the GLSL one) to the shader object and compile it
	glShaderSource(vertexShader, 1, &vShaderCode, NULL);
	glCompileShader(vertexShader);

	int successfullyCompiled; // An integer that checks if the vertex shader compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	// Returns a successful compilation of the vertex shader if it can compile successfully
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successfullyCompiled);

	// If the compilation failed, then return a log compilation error and explain the error
	if (!successfullyCompiled)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << compilationInformationLog << std::endl;
	}
}

void VertexShaderLoader::InitializeVertexObjects()
{
	// Texture coordinates in OpenGL go from 0,0 (bottom left) to 1,1 (top right)

	/*float textureCoordinates[] = {
		0.0f, 0.0f, // bottom left corner of the OpenGL window
		1.0f, 0.0f, // bottom right corner of the OpenGL window
		0.5f, 1.0f // top center of the OpenGL window
	};*/

	/* Apparently making the vertices a local variable makes the error go away compared to making it a global
	variable inside the class itself, even though the variable is private to every other class, including Main.cpp */

	// The vertices x, y, and z values must be in range of -1.0 and 1.0 for the object to render on the OpenGL window
	/* If one of the values goes below - 1.0 or above 1.0, the object will not be rendered on the window, meaning
	the object will go offscreen */

	// Initialize the first triangle's vertices and its colors to be at the middle of the OpenGL window
	float vertices[] = { 
		// positions      // colors         // texture coordinates

		0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.55f, 0.55f, // top right
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.55f, 0.45f, // bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.45f, 0.45f, // bottom left
		-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.45f, 0.55f // top right 
	};

	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	// Generate a vertex attribute object ID
	glGenVertexArrays(1, &VAO);

	// Generate the buffer ID here
	glGenBuffers(1, &VBO); // The & is a reference to the unsigned int of VBO and converts it to a GLuint pointer type

	// Generate the buffer ID here
	glGenBuffers(1, &EBO);

	// Bind the vertex array object using its ID
	glBindVertexArray(VAO);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// This binds the buffers more than once at the same time as long as they're different buffer types
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// Copies the previously defined vertex data into the buffer's memory
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Set the position attribute's location to 0 like our vertex shader GLSL file
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); // Position attribute location occurs at 1

	// Set the color attribute's location to 1 like our vertex shader GLSL file
	// 3 * sizeof(float) in the last argument below is the offset of the color which is 3 * of our position offset
	// The color offset is 12 bytes, and the position offset is at 0 and goes up to 3 bytes each time
	// Since we have 3 position values, the first color offset will occur at 12 bytes, hence why we multiply it by 3
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1); // Color attribute location occurs at 1

	// Let OpenGL know about the new vertex format using the newly created texture attribute
	// We have to adjust the stride parameter of the previous 2 vertex attributes to 8 * sizeof(float) as well
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// The first parameter takes in an input of how many textures we want to generate and store them in the integer
	glGenTextures(1, &texture1);

	// Then, we need to bind the textures to configure the currently bound texture on subsequent texture commands
	glBindTexture(GL_TEXTURE_2D, texture1);

	// Retrieving the texture colors using texture coordinates is called sampling
	// Texture sampling has a loose interpretation and we can sample the texture in different ways
	// To accomplish it, one must tell OpenGL how to sample the textures

	// Types of behaviour of texture wrapping in OpenGL: GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE and
	// GL_CLAMP_TO_BORDER

	/* GL_REPEAT is the default texture behaviour that repeats the textured image

	GL_MIRRORED_REPEAT is similar to GL_REPEAT except the textures will mirror the image on each repeat

	GL_CLAMP_TO_EDGE will clamp the texture image between 0 and 1. The result of higher texture coordinates will
	be clamped to the edge, which results in the image having a stretched edge pattern (makes the image look bigger)

	GL_CLAMP_TO_BORDER basically are texture coordinates that are out of range gives the user an option
	to specify a border color to clamp the texture image? */

	// The texture coordinates consists of a s, t and r (only for 3D textures) axes

	// First parameter passes in the texture target, second parameter passes in the axes that we want to set
	// And the third parameter passes in what we want the texture wrapping mode to be
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// If we choose GL_CLAMP_TO_BORDER, we should also specify a border color (r, g, b, a)
	/*float borderColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);*/

	// Texture flitering includes GL_NEAREST and GL_LINEAR

	/* GL_NEAREST selects the texture pixel that center is closest to the texture coordinate (e.g. closest to 0, 0.5)

	GL_LINEAR takes an interpolated value from the texture coordinate's neighouring texture pixels, approximating
	a color between the texture pixels (or texels)

	The smaller the distance from the texture coordinate to a texel's center, the more that texel's color contributes
	to the sampled color */

	/* GL_NEAREST results in a blocked pattern where we can clearly see the pixels that form the texture (produces a
	8-bit look that some developers prefer) compared to GL_LINEAR resulting in a more smooth pattern where the each
	pixel is less visible (produces a realistic output) */

	// Texture filtering can be set for magnifying (scaling up) and minifying (scaling downward) operations

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Nearest filtering for downward scaled textures
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear filtering for upscale textures

	// Mipmaps are collections of texture images where each subsequent (following) texture is twice as small compared
	// to the previous one

	/* After a certain distance threshold from the viewer, OpenGL will use a different mipmap texture that best suits
	the distance to the object. Because the object is far away, the object won't be visible to the user. OpenGL then
	samples the correct texture pixels and there is less cache memory involved when sampling that part of the mipmaps */

	// The filtering method between mipmap levels consist of GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST,
	// GL_NEAREST_MIPMAP_LINEAR and GL_LINEAR_MIPMAP_LINEAR

	/* GL_NEAREST_MIPMAP_NEAREST takes the nearest mipmap to match the pixel size and uses the nearest neighbor
	interpolation for texture sampling

	GL_LINEAR_MIPMAP_NEAREST takes the nearest mipmap level and samples that level using linear interpolation

	GL_NEAREST_MIPMAP_LINEAR linearly interpolates between the 2 mipmaps that most closely match the size of a pixel
	and samples the interpolated level via nearest neighbor interpolation

	GL_LINEAR_MIPMAP_LINEAR linearly interpolates between the 2 closest mipmaps and samples the interpolated level
	via nearest neighbor interpolation */

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* Setting the mipmap filter on a magnifying fliter has no effect on the mipmaps since they are primarily used
	when downscaling the textures as in texture magnification doesn't use mipmaps and passing in a mipmapping
	filtering option will give an OpenGL INVALID_CODE_ENUM error code */

	int width, height, nrChannels;

	stbi_set_flip_vertically_on_load(true);

	// The 3 int parameters of stbi_load include a width (x), height (y) and number of color channels (nrChannels)
	unsigned char* data = stbi_load("Textures/container.jpg", &width, &height, &nrChannels, 0);

	// After the texture ahs been binded, we can generate textures using the previously loaded image data
	// Textures are generated with glTexImage2D
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	else
	{
		std::cout << "This texture has failed to load!" << std::endl;
	}

	// Free the image memory after generating the texture and its corresponding mipmaps
	stbi_image_free(data);

	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	data = stbi_load("Textures/awesomeface.png", &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	else
	{
		std::cout << "This texture has failed to load!" << std::endl;
	}


	// Free the image memory after generating the texture and its corresponding mipmaps
	stbi_image_free(data);

	// Bind the texture before calling the glDrawElements function and it will automatically assign the texture to
	// the fragment shader's sampler
	// Bind the currently active texture here
	glActiveTexture(GL_TEXTURE0); // Active texture unit first
	glBindTexture(GL_TEXTURE_2D, texture1);

	glActiveTexture(GL_TEXTURE1); // Active texture unit first
	glBindTexture(GL_TEXTURE_2D, texture2);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}