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
}