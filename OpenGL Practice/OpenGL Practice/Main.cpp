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

	std::array <VertexShaderLoader*, 10> vertexShaderLoader;
	vertexShaderLoader = { 
		new VertexShaderLoader("LightingVertexShader.glsl"), 
		new VertexShaderLoader("LightCubeVertexShader.glsl"),
		new VertexShaderLoader("ModelVertexShader.glsl"),
		new VertexShaderLoader("DepthTestVertexShader.glsl"),
		new VertexShaderLoader("ColorVertexShader.glsl"),
		new VertexShaderLoader("TextureVertexShader.glsl"),
		new VertexShaderLoader("InversionVertexShader.glsl"),
		new VertexShaderLoader("GreyscaleVertexShader.glsl"),
		new VertexShaderLoader("KernelVertexShader.glsl"),
		new VertexShaderLoader("SkyboxVertexShader.glsl")
	};

	std::array <FragmentShaderLoader*, 10> fragmentShaderLoader;

	fragmentShaderLoader = {
		new FragmentShaderLoader("LightingFragmentShader.glsl"),
		new FragmentShaderLoader("LightCubeFragmentShader.glsl"),
		new FragmentShaderLoader("ModelFragmentShader.glsl"),
		new FragmentShaderLoader("DepthTestFragmentShader.glsl"),
		new FragmentShaderLoader("ColorFragmentShader.glsl"),
		new FragmentShaderLoader("TextureFragmentShader.glsl"),
		new FragmentShaderLoader("InversionFragmentShader.glsl"),
		new FragmentShaderLoader("GreyscaleFragmentShader.glsl"),
		new FragmentShaderLoader("KernelFragmentShader.glsl"),
		new FragmentShaderLoader("SkyboxFragmentShader.glsl")
	};

	// I don't need to make this shader program object a pointer because the constructor doesn't pass in anything
	ShaderProgram shaderProgram;

	// Just initialize the OpenGL window by filling in the right parameters below
	window.InitializeOpenGLwindow(1280, 960, "OpenGL Practice", NULL, NULL);

	for (unsigned int i = 0; i < vertexShaderLoader.size(); i++)
	{
		vertexShaderLoader[i]->InitializeVertexShaderLoader();
	}

	for (unsigned int j = 0; j < fragmentShaderLoader.size(); j++)
	{
		fragmentShaderLoader[j]->InitializeFragmentShaderLoader();
	}

	shaderProgram.~ShaderProgram();

	window.WindowStillRunning();

	// Return 0 is needed because the main function is of type int
	return 0;
}