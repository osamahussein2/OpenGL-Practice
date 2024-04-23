/*
APA references:

	Welcome to OpenGL. (n.d.) Learn OpenGL. 
		https://learnopengl.com/
*/

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>

void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);
void ProcessInput(GLFWwindow* window);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* openGLwindow = glfwCreateWindow(1280, 960, "OpenGL Practice", NULL, NULL);

	if (openGLwindow == NULL)
	{
		std::cout << "GLFW Window cannot be created!" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(openGLwindow);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "GLAD cannot be initialized!" << std::endl;
		return -1;
	}

	glViewport(0, 0, 1280, 960);

	glfwSetFramebufferSizeCallback(openGLwindow, FrameBufferSizeCallback);

	while (!glfwWindowShouldClose(openGLwindow))
	{
		ProcessInput(openGLwindow);

		glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(openGLwindow);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}