#include "Window.h"

Window::Window()
{
	vertexShaderLoader = new VertexShaderLoader("VertexShader.glsl");

	openGLwindow = NULL;

	shaderProgram = new ShaderProgram();
}

void Window::InitializeOpenGLwindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFW window to render it to the screen
	openGLwindow = glfwCreateWindow(width, height, title, monitor, share);

	// If our GLFWwindow is NULL, write to the console the window cannot be created and close down GLFW library
	if (openGLwindow == NULL)
	{
		std::cout << "GLFW Window cannot be created!" << std::endl;
		glfwTerminate();
		//return -1;
	}

	glfwMakeContextCurrent(openGLwindow);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "GLAD cannot be initialized!" << std::endl;
		//return -1;
	}

	glViewport(0, 0, 1280, 960);

	glfwSetFramebufferSizeCallback(openGLwindow, FrameBufferSizeCallback);
}

void Window::WindowStillRunning()
{
	/* While we don't want to close the GLFW window, process the input of our window, add our own background color
	for the window, clear the color buffer bit to render our color to the window, swap the window's buffers,
	process any events waiting for us to do something to it */
	while (!glfwWindowShouldClose(openGLwindow))
	{
		ProcessInput(openGLwindow);

		// Add our own color to the window
		glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

		// If I get rid of this, my window will be black because then we didn't clear any color buffer bit first before rendering
		glClear(GL_COLOR_BUFFER_BIT);

		shaderProgram->InitializeShaderProgram();
		vertexShaderLoader->InitializeVertexObjects();

		glfwSwapBuffers(openGLwindow); // Removing this will throw an exception error
		glfwPollEvents(); // Waits for any input by the user and processes it in real-time
	}

	// Close all GLFW-related stuff and perhaps terminate the whole program, maybe?
	glfwTerminate();
}

// Even though I don't know what a size callback is, I'm assuming this is supposed to get the viewport of the OpenGL window
void Window::FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void Window::ProcessInput(GLFWwindow* window)
{
	// Basically if we press the escape key while the window is running, it'll close the GLFW window for us
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	// Increase the texture visibility whenever we press the UP arrow key
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		shaderProgram->visibilityTextureValue += 0.001f;

		// If we go over 1, then set it equal to 1 so that we don't increase its visibility by a lot
		// Or even change its color abruptly, making the texture look out of whack with its real texture
		if (shaderProgram->visibilityTextureValue >= 1.0f)
		{
			shaderProgram->visibilityTextureValue = 1.0f;
		}
	}

	// Decrease the texture visibility whenever we press the DOWN arrow key
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		shaderProgram->visibilityTextureValue -= 0.001f;

		// If we go under 0, then set it equal to 0 so that we don't decrease its visibility by a lot
		// Or even change its color abruptly, making the texture look out of whack with its real texture
		if (shaderProgram->visibilityTextureValue <= 0.0f)
		{
			shaderProgram->visibilityTextureValue = 0.0f;
		}
	}
}