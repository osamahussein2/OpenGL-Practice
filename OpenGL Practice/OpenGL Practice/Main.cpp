/*
APA references (I'm trying to learn more about OpenGL using this tutorial below):

	Welcome to OpenGL. (n.d.) Learn OpenGL. 
		https://learnopengl.com/
*/

// Include my very own window, vertex shader loader, fragment shader loader and shader program classes
#include "Window.h"
#include "VertexShaderLoader.h"
#include "FragmentShaderLoader.h"
#include "ShaderProgram.h"

int main()
{
	// I don't need to make this window object a pointer because the constructor doesn't pass in anything
	Window window;

	std::array <VertexShaderLoader*, 5> vertexShaderLoader;
	vertexShaderLoader = { 
		new VertexShaderLoader("LightingVertexShader.glsl"), 
		new VertexShaderLoader("LightCubeVertexShader.glsl"),
		new VertexShaderLoader("ModelVertexShader.glsl"),
		new VertexShaderLoader("DepthTestVertexShader.glsl"),
		new VertexShaderLoader("ColorVertexShader.glsl")
	};

	std::array <FragmentShaderLoader*, 5> fragmentShaderLoader;

	fragmentShaderLoader = {
		new FragmentShaderLoader("LightingFragmentShader.glsl"),
		new FragmentShaderLoader("LightCubeFragmentShader.glsl"),
		new FragmentShaderLoader("ModelFragmentShader.glsl"),
		new FragmentShaderLoader("DepthTestFragmentShader.glsl"),
		new FragmentShaderLoader("ColorFragmentShader.glsl")
	};

	// I don't need to make this shader program object a pointer because the constructor doesn't pass in anything
	ShaderProgram shaderProgram;

	// Just initialize the OpenGL window by filling in the right parameters below
	window.InitializeOpenGLwindow(1280, 960, "OpenGL Practice", NULL, NULL);

	vertexShaderLoader[0]->InitializeVertexShaderLoader();
	vertexShaderLoader[1]->InitializeVertexShaderLoader();
	vertexShaderLoader[2]->InitializeVertexShaderLoader();
	vertexShaderLoader[3]->InitializeVertexShaderLoader();
	vertexShaderLoader[4]->InitializeVertexShaderLoader();

	fragmentShaderLoader[0]->InitializeFragmentShaderLoader();
	fragmentShaderLoader[1]->InitializeFragmentShaderLoader();
	fragmentShaderLoader[2]->InitializeFragmentShaderLoader();
	fragmentShaderLoader[3]->InitializeFragmentShaderLoader();
	fragmentShaderLoader[4]->InitializeFragmentShaderLoader();

	shaderProgram.~ShaderProgram();

	window.WindowStillRunning();

	// Return 0 is needed because the main function is of type int
	return 0;
}