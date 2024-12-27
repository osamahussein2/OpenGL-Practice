#include "AdvancedData.h"

/* A buffer in OpenGL, at its core, an object that manages a certain piece of GPU memory and nothing more. 
We give meaning to a buffer when binding it to a specific buffer target (e.g. GL_ELEMENT_ARRAY_BUFFER).
A buffer is only a vertex array buffer when we bind it to GL_ARRAY_BUFFER, but we can also bind it to 
GL_ELEMENT_ARRAY_BUFFER. OpenGL internally stores a reference to the buffer per target and, based on the target, processes
the buffer differently. */

/* We can fill the buffer's memory by calling glBufferData, which allocates (assigns) a piece of GPU memory and adds data
into this memory. If we were to pass NULL as its data argument, the function would only allocate memory and not fill it.
This is useful if we first want to reserve a specific amount of memory and come back to this buffer later on. */

/* Uniform buffer objects have several advantages over single uniforms. First, setting a lot of uniforms at once is faster 
than setting multiple uniforms one at a time. Second, if you want to change the same uniform over several shaders, it is 
much easier to change a uniform once in a uniform buffer. One last advantage that is not immediately apparent is that you 
can use a lot more uniforms in shaders using uniform buffer objects. OpenGL has a limit to how much uniform data it can 
handle which can be queried with GL_MAX_VERTEX_UNIFORM_COMPONENTS. When using uniform buffer objects, this limit is much 
higher. So whenever you reach a maximum number of uniforms (when doing skeletal animation for example) there’s always 
uniform buffer objects */

AdvancedData::AdvancedData()
{
	/*someData = {0.0f, 0.0f, 0.0f};
	pointer = nullptr;
	buffer = 0;*/

	// Create new Shader Program objects for the colored shaders
	redShader = new ShaderProgram();
	greenShader = new ShaderProgram();
	blueShader = new ShaderProgram();
	yellowShader = new ShaderProgram();
}

AdvancedData::~AdvancedData()
{
	/*someData = {0.0f, 0.0f, 0.0f};
	pointer = nullptr;
	buffer = 0;*/

	// Deallocate the cube's VAO and VBO at the end of runtime
	glDeleteBuffers(1, &cubeVBO);
	glDeleteVertexArrays(1, &cubeVAO);
}

void AdvancedData::InitializeCubeVertices()
{
	// Initialize the color shaders
	redShader->InitializeShaderProgram(new VertexShaderLoader("AdvancedGLSLVertexShader.glsl"), 
		new FragmentShaderLoader("RedFragmentShader.glsl"));

	greenShader->InitializeShaderProgram(new VertexShaderLoader("AdvancedGLSLVertexShader.glsl"),
		new FragmentShaderLoader("GreenFragmentShader.glsl"));

	blueShader->InitializeShaderProgram(new VertexShaderLoader("AdvancedGLSLVertexShader.glsl"),
		new FragmentShaderLoader("BlueFragmentShader.glsl"));

	yellowShader->InitializeShaderProgram(new VertexShaderLoader("AdvancedGLSLVertexShader.glsl"),
		new FragmentShaderLoader("YellowFragmentShader.glsl"));

	// Set the cube vertices
	cubeVertices = {
		// positions         
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
}

void AdvancedData::InitializeBufferObject()
{
	/* To set a shader uniform block to a specific binding point we call glUniformBlockBinding that takes a program object, 
	a uniform block index, and the binding point to link to. The uniform block index is a location index of the defined 
	uniform block in the shader. This can be retrieved via a call to glGetUniformBlockIndex that accepts a program object 
	and the name of the uniform block */

	// Configure a uniform buffer object by getting the relevant block indices from the shader
	unsigned int uniformBlockIndexRed = glGetUniformBlockIndex(redShader->shaderProgram, "Matrices");
	unsigned int uniformBlockIndexGreen = glGetUniformBlockIndex(greenShader->shaderProgram, "Matrices");
	unsigned int uniformBlockIndexBlue = glGetUniformBlockIndex(blueShader->shaderProgram, "Matrices");
	unsigned int uniformBlockIndexYellow = glGetUniformBlockIndex(yellowShader->shaderProgram, "Matrices");

	// Link each shader's uniform block to this uniform binding point
	glUniformBlockBinding(redShader->shaderProgram, uniformBlockIndexRed, 0);
	glUniformBlockBinding(greenShader->shaderProgram, uniformBlockIndexGreen, 0);
	glUniformBlockBinding(blueShader->shaderProgram, uniformBlockIndexBlue, 0);
	glUniformBlockBinding(yellowShader->shaderProgram, uniformBlockIndexYellow, 0);

	// Create the uniform buffer
	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// The function glBindbufferBase expects a target, a binding point index and a uniform buffer object

	/* The function glBindBufferRange that expects an extra offset and size parameter - this way you can bind only a 
	specific range of the uniform buffer to a binding point. Using glBindBufferRange you could have multiple different 
	uniform blocks linked to a single uniform buffer object */

	// Define the range of the buffer that links to a uniform binding point
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

	// Store the first half of the uniform buffer with the projection matrix once before rendering loop
	glm::mat4 projectionMatrix = glm::perspective(45.0f, float(800 / 600), 0.1f, 100.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMatrix));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void AdvancedData::LoadCubesInfo()
{
	// Set the view and projection matrix in the uniform block (perform only once per loop iteration)
	glm::mat4 viewMatrix = Camera::CameraLookAt();

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

	/* Add all the data as a single byte array, or update parts of the buffer whenever we feel like it using 
	glBufferSubData. To update the uniform variable boolean we could update the uniform buffer object as follows*/
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(viewMatrix));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindVertexArray(cubeVAO);
}

void AdvancedData::DrawRedCube()
{
	glUseProgram(redShader->shaderProgram);

	// RED cube
	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.75f, 0.75f, 0.0f)); // move top-left

	glUniformMatrix4fv(glGetUniformLocation(redShader->shaderProgram, "modelMatrix"), 1, GL_FALSE,
		glm::value_ptr(modelMatrix));

	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void AdvancedData::DrawGreenCube()
{
	glUseProgram(greenShader->shaderProgram);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.75f, 0.75f, 0.0f)); // move top-right

	glUniformMatrix4fv(glGetUniformLocation(greenShader->shaderProgram, "modelMatrix"), 1, GL_FALSE,
		glm::value_ptr(modelMatrix));

	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void AdvancedData::DrawBlueCube()
{
	glUseProgram(blueShader->shaderProgram);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.75f, -0.75f, 0.0f)); // move bottom-right

	glUniformMatrix4fv(glGetUniformLocation(blueShader->shaderProgram, "modelMatrix"), 1, GL_FALSE,
		glm::value_ptr(modelMatrix));

	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void AdvancedData::DrawYellowCube()
{
	glUseProgram(yellowShader->shaderProgram);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.75f, -0.75f, 0.0f)); // move bottom-left

	glUniformMatrix4fv(glGetUniformLocation(yellowShader->shaderProgram, "modelMatrix"), 1, GL_FALSE,
		glm::value_ptr(modelMatrix));

	glDrawArrays(GL_TRIANGLES, 0, 36);
}

/*void AdvancedData::MapBuffer()
{
	/* Instead of filling the entire buffer with one function call we can also fill specific regions of the buffer by calling
	glBufferSubData. This function expects a buffer target, an offset, the size of the data and the actual data as its 
	arguments. What's new with this function is that we can now give an offset that specifies from where we want to fill the
	buffer. This allows us to insert/update only certain parts of the buffer's memory. Do note that the buffer should have
	enough allocated memory so a call to glBufferData is necessary before calling glBufferSubData on the buffer. */

	/* Yet another method for getting data into a buffer is to ask for a pointer to the buffer's memory and directly
	copy the data in memory yourself. By calling glMapBuffer, OpenGL returns a pointer to the currently bound buffer's 
	memory for us to operate on */

	/*someData = {0.5f, 1.0f, -0.35f};

	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	// Using glMapBufferis useful for directly mapping data to a buffer, without first storing it in temporary memory
	// Like reading data from a file and copying it into the buffer's memory

	// Initialize the pointer to a mapped written buffer
	pointer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	// Copy the data into the memory
	memcpy(pointer, &someData, sizeof(someData));

	/* By unmapping the pointer becomes invalid and the function returns GL_TRUE if OpenGL was able to map the data
	successfully to the buffer */

	// Make sure we unmap the buffer after we're done with the pointer object
	//glUnmapBuffer(GL_ARRAY_BUFFER);

	/* Using glVertexAttribPointer we were able to specify the attribute layout of the vertex array buffer’s content. 
	Within the vertex array buffer we interleaved (mixed) the attributes; that is, we placed the position, normal 
	and/or texture coordinates next to each other in memory for each vertex */

	/* When loading vertex data from file you generally retrieve an array of positions, an array of
	normals and/or an array of texture coordinates. It may cost some effort to combine these arrays into
	one large array of interleaved data. Taking the batching approach is then an easier solution that we
	can easily implement using glBufferSubData */

	//positions = { 0.5f, 0.5f, 0.5f };
	//normals = { 0.0f, 0.0f, -1.0f };
	//textureCoordinates = { 0.0f, 0.0f };

	/* Fill the batched buffer. This way we can directly transfer the attribute arrays as a whole into the buffer 
	without first having to process them */
	/*glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), &positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), &normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), sizeof(textureCoordinates), &textureCoordinates);*/

	/* Note that the stride parameter is equal to the size of the vertex attribute, since the next vertex attribute 
	vector can be found directly after its 3 (or 2) components */
	/*glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(sizeof(positions)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)(sizeof(positions) + sizeof(normals)));*/

	/* Once your buffers are filled with data you may want to share that data with other buffers or perhaps
	copy the buffer’s content into another buffer. The function glCopyBufferSubData allows us to
	copy the data from one buffer to another buffer with relative ease */
	
	/* void glCopyBufferSubData(GLenum readtarget, GLenum writetarget, GLintptr readoffset, GLintptr writeoffset,
	GLsizeiptr size); */

	/* The readtarget and writetarget parameters expect to give the buffer targets that we
	want to copy from and to. We could for example copy from a VERTEX_ARRAY_BUFFER buffer to
	a VERTEX_ELEMENT_ARRAY_BUFFER buffer by specifying those buffer targets as the read and
	write targets respectively. The buffers currently bound to those buffer targets will then be affected */

	/* We can’t bind two buffers at the same time to the same buffer target. For this reason, and
	this reason alone, OpenGL gives us two more buffer targets called GL_COPY_READ_BUFFER and
	GL_COPY_WRITE_BUFFER. We then bind the buffers of our choice to these new buffer targets
	and set those targets as the readtarget and writetarget argument */

	/* glCopyBufferSubData then reads data of a given size from a given readoffset and writes it into the writetarget 
	buffer at writeoffset */

	/*array<unsigned int, 2> VBOs{};

	unsigned int dataSize = sizeof(vec3) + sizeof(vec3) + sizeof(vec2);

	glBindBuffer(GL_COPY_READ_BUFFER, VBOs[0]);
	glBindBuffer(GL_COPY_WRITE_BUFFER, VBOs[1]);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, dataSize);

	// Binding the writetarget buffer to one of the new buffer target types
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBindBuffer(GL_COPY_WRITE_BUFFER, VBOs[1]);
	glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, dataSize);

}*/