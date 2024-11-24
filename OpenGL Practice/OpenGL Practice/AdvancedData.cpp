#include "AdvancedData.h"

/* A buffer in OpenGL, at its core, an object that manages a certain piece of GPU memory and nothing more. 
We give meaning to a buffer when binding it to a specific buffer target (e.g. GL_ELEMENT_ARRAY_BUFFER).
A buffer is only a vertex array buffer when we bind it to GL_ARRAY_BUFFER, but we can also bind it to 
GL_ELEMENT_ARRAY_BUFFER. OpenGL internally stores a reference to the buffer per target and, based on the target, processes
the buffer differently. */

/* We can fill the buffer's memory by calling glBufferData, which allocates (assigns) a piece of GPU memory and adds data
into this memory. If we were to pass NULL as its data argument, the function would only allocate memory and not fill it.
This is useful if we first want to reserve a specific amount of memory and come back to this buffer later on. */

AdvancedData::AdvancedData()
{
	someData = { 0.0f, 0.0f, 0.0f };
	pointer = nullptr;
	buffer = 0;
}

AdvancedData::~AdvancedData()
{
	someData = { 0.0f, 0.0f, 0.0f };
	pointer = nullptr;
	buffer = 0;
}

void AdvancedData::MapBuffer()
{
	/* Instead of filling the entire buffer with one function call we can also fill specific regions of the buffer by calling
	glBufferSubData. This function expects a buffer target, an offset, the size of the data and the actual data as its 
	arguments. What's new with this function is that we can now give an offset that specifies from where we want to fill the
	buffer. This allows us to insert/update only certain parts of the buffer's memory. Do note that the buffer should have
	enough allocated memory so a call to glBufferData is necessary before calling glBufferSubData on the buffer. */

	/* Yet another method for getting data into a buffer is to ask for a pointer to the buffer's memory and directly
	copy the data in memory yourself. By calling glMapBuffer, OpenGL returns a pointer to the currently bound buffer's 
	memory for us to operate on */

	someData = { 0.5f, 1.0f, -0.35f };

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
	glUnmapBuffer(GL_ARRAY_BUFFER);

	/* Using glVertexAttribPointer we were able to specify the attribute layout of the vertex array buffer’s content. 
	Within the vertex array buffer we interleaved (mixed) the attributes; that is, we placed the position, normal 
	and/or texture coordinates next to each other in memory for each vertex */

	/* When loading vertex data from file you generally retrieve an array of positions, an array of
	normals and/or an array of texture coordinates. It may cost some effort to combine these arrays into
	one large array of interleaved data. Taking the batching approach is then an easier solution that we
	can easily implement using glBufferSubData */

	positions = { 0.5f, 0.5f, 0.5f };
	normals = { 0.0f, 0.0f, -1.0f };
	textureCoordinates = { 0.0f, 0.0f };

	/* Fill the batched buffer. This way we can directly transfer the attribute arrays as a whole into the buffer 
	without first having to process them */
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), &positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), &normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), sizeof(textureCoordinates), &textureCoordinates);

	/* Note that the stride parameter is equal to the size of the vertex attribute, since the next vertex attribute 
	vector can be found directly after its 3 (or 2) components */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(sizeof(positions)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)(sizeof(positions) + sizeof(normals)));

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

	array<unsigned int, 2> VBOs{};

	unsigned int dataSize = sizeof(vec3) + sizeof(vec3) + sizeof(vec2);

	glBindBuffer(GL_COPY_READ_BUFFER, VBOs[0]);
	glBindBuffer(GL_COPY_WRITE_BUFFER, VBOs[1]);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, dataSize);

	// Binding the writetarget buffer to one of the new buffer target types
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBindBuffer(GL_COPY_WRITE_BUFFER, VBOs[1]);
	glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, dataSize);

}