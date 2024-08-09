#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include "VertexShaderLoader.h"
#include "ShaderProgram.h"

class Window
{
public:
	// Initialize the window's variables to be NULL first before we do anything with it
	Window();

	// Initialize OpenGL window here
	void InitializeOpenGLwindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);

	void WindowStillRunning();

	// Make this function static to use it inside the glfwSetFramebufferSizeCallback function

	/* If I don't make this static, it'll give an error that it cannot convert this function for the Window class
	to a parameter type of GLFWframebuffersizefun */
	static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

	// Get the keyboard input whenever we want to close the window
	void ProcessInput(GLFWwindow* window);

private:
	VertexShaderLoader* vertexShaderLoader;
	ShaderProgram* shaderProgram;
	GLFWwindow* openGLwindow;
};

#endif