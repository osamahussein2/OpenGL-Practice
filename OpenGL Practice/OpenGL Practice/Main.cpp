/*
APA references (I'm trying to learn more about OpenGL using this tutorial below):

	Welcome to OpenGL. (n.d.) Learn OpenGL. 
		https://learnopengl.com/
*/

// Include my very own window, vertex shader loader classes
#include "Window.h"
#include "VertexShaderLoader.h"
#include "FragmentShaderLoader.h"
#include "ShaderProgram.h"

int main()
{
	// I don't need to make this window object a pointer because I don't have a constructor for it
	Window window;

	VertexShaderLoader* vertexShaderLoader;
	vertexShaderLoader = new VertexShaderLoader();

	FragmentShaderLoader* fragmentShaderLoader;
	fragmentShaderLoader = new FragmentShaderLoader();

	ShaderProgram* shaderProgram;
	shaderProgram = new ShaderProgram();

	// Just initialize the OpenGL window by filling in the right parameters below
	window.InitializeOpenGLwindow(1280, 960, "OpenGL Practice", NULL, NULL);

	vertexShaderLoader->InitializeVertexShaderLoader();
	vertexShaderLoader->InitializeVertexShaderLoader2();
	vertexShaderLoader->InitializeVertexShaderLoader3();
	fragmentShaderLoader->InitializeFragmentShaderLoader();
	fragmentShaderLoader->InitializeFragmentShaderLoader2();
	fragmentShaderLoader->InitializeFragmentShaderLoader3();
	shaderProgram->InitializeShaderProgram();
	shaderProgram->InitializeShaderProgram2();
	shaderProgram->InitializeShaderProgram3();
	shaderProgram->~ShaderProgram();

	window.WindowStillRunning();

	// Return 0 is needed because the main function is of type int
	return 0;
}