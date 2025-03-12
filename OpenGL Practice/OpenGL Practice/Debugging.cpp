#include "Debugging.h"

/* The moment you incorrectly use OpenGL (like configuring a buffer without first binding any) it will take notice and 
generate one or more user error flags behind the scenes. We can query these error flags using a function named glGetError 
that checks the error flag(s) set and returns an error value if OpenGL got misused: GLenum glGetError(); */

/*
	The moment glGetError is called, it returns either an error flag or no error at all. The error codes that glGetError can 
	return are listed here:

	GL_NO_ERROR - code 0, no user error reported since last call to glGetError.
	GL_INVALID_ENUM - code 1280, set when an enumeration parameter is not legal.
	GL_INVALID_VALUE - code 1281, set when a value parameter is not legal.
	GL_INVALID_OPERATION - code 1282, set when the state for a command is not legal for its given parameters.
	GL_STACK_OVERFLOW - code 1283, set when a stack pushing operation causes a stack overflow.
	GL_STACK_UNDERFLOW - code 1284, set when a stack popping operation occurs while the stack is at its lowest point.
	GL_OUT_OF_MEMORY - code 1285, set when a memory allocation operation cannot allocate (enough) memory.
	GL_INVALID_FRAMEBUFFER_OPERATION - code 1286, set when reading or writing to a framebuffer that is not complete.
*/

Debugging* Debugging::debuggingInstance = NULL;

Debugging::Debugging() : debuggingShader(new ShaderProgram()), cubeVAO(0), cubeVBO(0), cubeVertices{NULL}, texture(0),
projection(mat4(0.0f))
{
}

Debugging::~Debugging()
{
	delete debuggingShader;

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);

	for (unsigned int i = 0; i < cubeVertices.size(); i++)
	{
		cubeVertices[i] = NULL;
	}

	glDeleteTextures(1, &texture);

	projection = mat4(NULL);
}

Debugging* Debugging::Instance()
{
	if (debuggingInstance == NULL)
	{
		debuggingInstance = new Debugging();
		return debuggingInstance;
	}

	return debuggingInstance;
}

void Debugging::InitializeDebugging()
{
	/* The great thing about glGetError is that it makes it relatively easy to pinpoint where any error may be and to validate 
	the proper use of OpenGL (LEARNING PURPOSES ONLY) */
	/*glBindTexture(GL_TEXTURE_2D, tex);
	cout << glGetError() << endl; // returns 0 (no error)

	glTexImage2D(GL_TEXTURE_3D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	cout << glGetError() << endl; // returns 1280 (invalid enum)

	glGenTextures(-5, &textures);
	cout << glGetError() << endl; // returns 1281 (invalid value)
	cout << glGetError() << endl; // returns 0 (no error)

	glBindBuffer(GL_VERTEX_ARRAY, vbo);
	glCheckError();*/

	/* Aside from reading messages, we can also push messages to the debug output system with glDebugMessageInsert:
	glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_MEDIUM, -1, 
	"error message here"); */

	/* This is especially useful if you’re hooking into other application or OpenGL code that makes use of a debug output 
	context */

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		/* Tell OpenGL to enable debug output. The glEnable(GL_DEBUG_SYNCRHONOUS) call tells OpenGL to directly call the
		callback function the moment an error occurred */

		// initialize debug output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);

		/* Filter the type(s) of errors you’d like to receive a message from. Don't filter on any of the sources, types, or
		severity rates but only show messages from the OpenGL API, that are errors, and have a high severity */
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	debuggingShader->InitializeShaderProgram(new VertexShaderLoader("DebuggingVertexShader.glsl"), 
		new FragmentShaderLoader("DebuggingFragmentShader.glsl"));

	cubeVertices =
	{
		// back face
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, // bottom-left
		0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // top-right
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f, // bottom-right         
		0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // top-right
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, // bottom-left
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f, // top-left
	   // front face
	  -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // bottom-left
	   0.5f, -0.5f,  0.5f,  1.0f,  0.0f, // bottom-right
	   0.5f,  0.5f,  0.5f,  1.0f,  1.0f, // top-right
	   0.5f,  0.5f,  0.5f,  1.0f,  1.0f, // top-right
	  -0.5f,  0.5f,  0.5f,  0.0f,  1.0f, // top-left
	  -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // bottom-left
	  // left face
	  -0.5f,  0.5f,  0.5f, -1.0f,  0.0f, // top-right
	  -0.5f,  0.5f, -0.5f, -1.0f,  1.0f, // top-left
	  -0.5f, -0.5f, -0.5f, -0.0f,  1.0f, // bottom-left
	 -0.5f, -0.5f, -0.5f, -0.0f,  1.0f, // bottom-left
	 -0.5f, -0.5f,  0.5f, -0.0f,  0.0f, // bottom-right
	 -0.5f,  0.5f,  0.5f, -1.0f,  0.0f, // top-right
	 // right face
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // top-left
	 0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // top-right         
	 0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // top-left
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // bottom-left     
	 // bottom face
	-0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // top-right
	 0.5f, -0.5f, -0.5f,  1.0f,  1.0f, // top-left
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f, // bottom-left
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // bottom-right
	-0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // top-right
	// top face
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f, // top-left
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // bottom-right
	0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // top-right     
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // bottom-right
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f, // top-left
   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f  // bottom-left        
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Load cube texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height, nrComponents;
	unsigned char* data = stbi_load("Textures/Wood.png", &width, &height, &nrComponents, 0);

	if (data)
	{
		//glTexImage2D(GL_FRAMEBUFFER, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	projection = perspective(radians(45.0f), float(1280 / 960), 0.1f, 10.0f);
}

void Debugging::RenderDebugging()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(debuggingShader->shaderProgram);

	float rotationSpeed = 10.0f;
	float angle = (float)glfwGetTime() * rotationSpeed;

	glUniform1i(glGetUniformLocation(debuggingShader->shaderProgram, "tex"), 0);

	glUniformMatrix4fv(glGetUniformLocation(debuggingShader->shaderProgram, "projection"), 1, GL_FALSE,
		value_ptr(projection));

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0, 0.0f, -2.5));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 1.0f, 1.0f));

	glUniformMatrix4fv(glGetUniformLocation(debuggingShader->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));

	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(cubeVAO);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

GLenum Debugging::CheckOpenGLerror(const char* file, int line)
{
	GLenum errorCode;

	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		string error;

		switch (errorCode)
		{
		case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW: error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW: error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;

		default: break;
		}

		cout << error << " | " << file << " (" << line << ")" << endl;

	}

	return errorCode;
}

void Debugging::glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, 
	const char* message, const void* userParam)
{
	/* Whenever debug output detects an OpenGL error, it will call this callback function and we’ll be able to print out a 
	large deal of information regarding the OpenGL error. Note that we ignore a few error codes that tend to not really 
	display anything useful (like 131185 in NVidia drivers that tells us a buffer was successfully created) */
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	cout << "---------------" << endl;
	cout << "Debug message (" << id << "): " << message << endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:
		cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:
		cout << "Source: Other"; break;
	} cout << endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:
		cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:
		cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:
		cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:
		cout << "Type: Other"; break;
	}  cout << endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:
		cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		cout << "Severity: notification"; break;
	} cout << endl;
	cout << endl;
}
