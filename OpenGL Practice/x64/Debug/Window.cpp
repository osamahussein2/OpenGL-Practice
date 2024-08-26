#include "Window.h"

Window::Window()
{
	vertexShaderLoader = new VertexShaderLoader("VertexShader.glsl");

	openGLwindow = NULL;

	shaderProgram = new ShaderProgram();
	camera = new Camera();

	cameraMoveSpeed = 0.0f;

	deltaTime = 0.0f;
	lastFrame = 0.0f;
	currentFrame = 0.0f;
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
		currentFrame = glfwGetTime(); // Get the current time

		// deltaTime is the time between current frame and last frame
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame; // Get the time of the last frame

		ProcessInput(openGLwindow);

		// OpenGL stores all its depth information in a z-buffer, also known as depth buffering

		/* The depth is stored within each fragment (as the fragment's z value) and whenever the fragment wants to 
		output its color, OpenGL compares its depth values with the z-buffer. If the current fragment is behind the 
		other fragment it is discarded, otherwise it's overwritten. This process is called depth testing. */
		glEnable(GL_DEPTH_TEST);

		// Add our own color to the window
		glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

		// If I get rid of this, my window will be black because then we didn't clear any color buffer bit first before rendering
		// Since we're also using depth buffer, we need to clear the depth buffer before each render iteration
		// Otherwise, the depth information of the previous frame stays in the buffer (and the cube won't render at all)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderProgram->InitializeShaderProgram();
		vertexShaderLoader->InitializeVertexObjects();

		// Call the second texture initialization function here to render the second container and scale it overtime
		//shaderProgram->InitializeSecondTexture();

		// Call the 3D object initialization function to render 3D objects onto the OpenGL window
		// The 800.0f/ 600.0f below also equals to 4.0f / 3.0f which is what most graphics aspect ratios are defaulted to
		shaderProgram->Initialize3Dobjects(800.0f / 600.0f, 0.1f, 100.0f);

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

	// Set the camera's moving speed to 2.5 times by the deltaTime when our OpenGL program is still running
	cameraMoveSpeed = 2.5f * deltaTime;

	// Move the camera around with the WASD keys
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		// Move the camera towards the screen when W is pressed
		Camera::cameraPosition += cameraMoveSpeed * Camera::cameraFront;

		// I managed to get the up y-axis movement working here but I commented it out for the sake of the tutorial
		//Camera::cameraPosition += cameraMoveSpeed * glm::normalize(glm::cross(Camera::cameraRight, Camera::cameraFront));
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		// Move the camera away from the screen when S key is pressed
		Camera::cameraPosition -= cameraMoveSpeed * Camera::cameraFront;
		
		// I managed to get the down y-axis movement working here but I commented it out for the sake of the tutorial
		//Camera::cameraPosition -= cameraMoveSpeed * glm::normalize(glm::cross(Camera::cameraRight, Camera::cameraFront));
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		// Move the camera left when A key is pressed
		Camera::cameraPosition -= glm::normalize(glm::cross(Camera::cameraFront, Camera::cameraUp)) * cameraMoveSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		// Move the camera right when D key is pressed
		Camera::cameraPosition += glm::normalize(glm::cross(Camera::cameraFront, Camera::cameraUp)) * cameraMoveSpeed;
	}
}