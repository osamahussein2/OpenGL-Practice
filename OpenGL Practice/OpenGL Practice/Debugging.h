#pragma once

#include <array>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "stb_image.h"

#include "ShaderProgram.h"

class Debugging
{
public:
	~Debugging();

	static Debugging* Instance();

	void InitializeDebugging();
	void RenderDebugging();

private:
	Debugging();

	GLenum CheckOpenGLerror(const char* file, int line);

	static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
		const char* message, const void* userParam);

	static Debugging* debuggingInstance;

	ShaderProgram* debuggingShader;

	unsigned int cubeVAO, cubeVBO, texture;

	array<float, 180> cubeVertices;

	mat4 projection;

	//unsigned int tex, textures;
	//char* data;

	//unsigned int vbo;
};

/* Preprocessor directives __FILE__ and __LINE__ variables get replaced during compile time with the respective file and line
they were compiled in. If we decide to stick a large number of these glCheckError calls in our codebase it’s helpful to more
precisely know which glCheckError call returned the error */
#define glCheckError() CheckOpenGLerror(__FILE__, __LINE__)

typedef Debugging DebuggingTime;