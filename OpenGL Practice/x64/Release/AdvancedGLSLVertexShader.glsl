#version 330 core

layout (location = 0) in vec3 position;
//layout (location = 1) in vec2 textureCoord;

/* Variables in a uniform block can be directly accessed without the block name as a prefix. Then we store these matrix
values in a buffer somewhere in the OpenGL code and each shader that declares this uniform block has access to the 
matrices. layout (std140) means that the currently defined uniform block uses a specific memory layout for its content;
this statement sets the uniform block layout */

/* The content of a uniform block is stored in a buffer object. Because this piece of memory holds no information on
what kind of data it holds, we need to tell OpenGL what parts of the memory correspond to which uniform variables in 
the shader. */

layout (std140) uniform Matrices
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
};

uniform mat4 modelMatrix;


/* To organize the in and out shader variables, GLSL has interface blocks that allows us to group variables together. 
The declaration of such an interface block looks a lot like a struct declaration, except that it is now declared using 
an in or out keyword based on the block being an input or an output block */

/*out VS_OUT
{
	vec2 textureCoordinate;
} vs_out; */

void main()
{
	/* gl_Position and gl_PointSize are output variables since their value is read as output from the vertex shader,
	which we can influence the result by writing to them */

	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

	/* By setting the point size to the clip-space z position, the point size will increase or decrease depending on
	the viewer's distance from the vertices (e.g. the point size should increase the further we are from the vertices
	as the viewer) */
	//gl_PointSize = gl_Position.z;

	//vs_out.textureCoordinate = textureCoord;

	/* gl_VertexID is an input variable, that we can only read from. The integer variable gl_VertexID holds the current ID
	of the vertex we’re drawing. When doing indexed rendering (with glDrawElements) this variable holds the current index 
	of the vertex we’re drawing. When drawing without indices (via glDrawArrays) this variable holds the number of the 
	currently processed vertex since the start of the render call. */
}