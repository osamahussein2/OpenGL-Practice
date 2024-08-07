/*
APA references (I'm trying to learn more about OpenGL using this tutorial below):

	Welcome to OpenGL. (n.d.) Learn OpenGL. 
		https://learnopengl.com/
*/

// Include my very own window, vertex shader loader, fragment shader loader,shader program, and texture classes
#include "Window.h"
#include "VertexShaderLoader.h"
#include "FragmentShaderLoader.h"
#include "ShaderProgram.h"

int main()
{
	// I don't need to make this window object a pointer because the constructor doesn't pass in anything
	Window window;

	VertexShaderLoader* vertexShaderLoader;
	vertexShaderLoader = new VertexShaderLoader("VertexShader.glsl");

	FragmentShaderLoader* fragmentShaderLoader;
	fragmentShaderLoader = new FragmentShaderLoader("FragmentShader.glsl");

	// I don't need to make this shader program object a pointer because the constructor doesn't pass in anything
	ShaderProgram shaderProgram;

	// Just initialize the OpenGL window by filling in the right parameters below
	window.InitializeOpenGLwindow(1280, 960, "OpenGL Practice", NULL, NULL);

	vertexShaderLoader->InitializeVertexShaderLoader();
	fragmentShaderLoader->InitializeFragmentShaderLoader();
	shaderProgram.InitializeShaderProgram();
	shaderProgram.~ShaderProgram();

	window.WindowStillRunning();

	// Return 0 is needed because the main function is of type int
	return 0;
}