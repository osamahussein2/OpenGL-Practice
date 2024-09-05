#include "Window.h"

float Window::lastPositionX = 400;
float Window::lastPositionY = 300;

// Initialize the first time mouse input to true since the mouse cursor will be immediately focused in OpenGL window
extern bool firstTimeMouseReceivesInput = true;

Window::Window()
{
	vertexShaderLoader =
	{ 
		new VertexShaderLoader("LightingVertexShader.glsl"), 
		new VertexShaderLoader("LightCubeVertexShader.glsl")
	};

	fragmentShaderLoader =
	{
		new FragmentShaderLoader("LightingFragmentShader.glsl"),
		new FragmentShaderLoader("LightCubeFragmentShader.glsl")
	};

	openGLwindow = NULL;

	shaderProgram = new ShaderProgram();
	camera = new Camera();

	cameraMoveSpeed = 0.0f;

	deltaTime = 0.0f;
	lastFrame = 0.0f;
	currentFrame = 0.0f;

	lastPositionX = 0.0f;
	lastPositionY = 0.0f;

	// To initialize an enum, just use the enum's name along with open parentheses
	cameraMovement = CameraMovement();

	color = new Color();
	lighting = new Lighting();
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

		// Tell GLFW to hide the mouse cursor and capture it

		/* Capturing a cursor means that, once the application has focus, the mouse cursor stays within the center of the 
		window (unless if the application loses focus or quits )*/
		glfwSetInputMode(openGLwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		/* As soon as we register the callback function with GLFW each time the mouse moves while the window has focus on
		the cursor, this MouseCallback function below will get called. */
		glfwSetCursorPosCallback(openGLwindow, MouseCallback);

		// Register the mouse scroll callback every time we move the mouse scroll wheel
		glfwSetScrollCallback(openGLwindow, MouseScrollCallback);

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

		shaderProgram->InitializeShaderProgram(vertexShaderLoader[0], fragmentShaderLoader[0]);
		vertexShaderLoader[0]->InitializeVertexObjects();

		// Call the 3D object initialization function to render 3D objects onto the OpenGL window
		// The 800.0f/ 600.0f below also equals to 4.0f / 3.0f which is what most graphics aspect ratios are defaulted to
		shaderProgram->InitializeCubeColor(800.0f / 600.0f, 0.1f, 100.0f);

		shaderProgram->InitializeShaderProgram(vertexShaderLoader[1], fragmentShaderLoader[1]);
		vertexShaderLoader[1]->InitializeVertexObjects();

		// Call the second texture initialization function here to render the second container and scale it overtime
		//shaderProgram->InitializeSecondTexture();

		shaderProgram->InitializeLightColor(800.0f / 600.0f, 0.1f, 100.0f);

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

// To calculate the pitch and yaw values, we must tell GLFW to listen to mouse-movement events
void Window::MouseCallback(GLFWwindow* window, double positionX, double positionY)
{
	// If the mouse receives input for the first time, update the initial mouse positions to the x and y positions
	if (firstTimeMouseReceivesInput == true)
	{
		lastPositionX = positionX;
		lastPositionY = positionY;

		firstTimeMouseReceivesInput = false; // Make this false afterwards to prevent long movement jumps
	}

	/* When handling mouse input for a fly style camera, we have to perform 4 steps before the camera's direction vector
	will be fully calculated to produce the final output. */

	/* 
		1. Calculate the mouse's offset since the last frame
		2. Add the offset values to the camera's yaw and pitch values
		3. Add some constraints to the minimum or maximum yaw and pitch values
		4. Calculate the direction vector
	*/

	/* In order to find the mouse's offset from last frame, we have to store the last mouse x and y positions in the 
	application. */
	float mouseOffsetX = positionX - lastPositionX;
	float mouseOffsetY = lastPositionY - positionY; // This has to be reversed since y ranges bottom to top

	lastPositionX = positionX;
	lastPositionY = positionY;

	/* We need to multiply the mosue offset values by the mouse sensitivity value here or else the mouse movement would
	be too strong. */
	const float mouseSensitivity = 0.1f;
	mouseOffsetX *= mouseSensitivity;
	mouseOffsetY *= mouseSensitivity;

	// Add the mouse offset x value to yaw and the mouse offset y value to pitch
	Camera::yaw += mouseOffsetX;
	Camera::pitch += mouseOffsetY;

	/* We need to restrict the pitch value not to exceed 89 and not to go below -89 to prevent the LookAt view matrix
	from flipping. */
	if (Camera::pitch >= 89.0f)
	{
		Camera::pitch = 89.0f;
	}

	else if (Camera::pitch <= -89.0f)
	{
		Camera::pitch = -89.0f;
	}

	Camera::cameraDirection = glm::vec3(cos(glm::radians(Camera::yaw)) * cos(glm::radians(Camera::pitch)),
		sin(glm::radians(Camera::pitch)),
		sin(glm::radians(Camera::yaw) * cos(glm::radians(Camera::pitch))));

	Camera::cameraFront = glm::normalize(Camera::cameraDirection);
}

void Window::MouseScrollCallback(GLFWwindow* window, double offsetX, double offsetY)
{
	// Decrease the field of view by the mouse y offset each time
	Camera::fieldOfView -= offsetY;

	// If the field of view of the camera is zoomed in by 1.0 or less, set the field of view equal to 1.0
	if (Camera::fieldOfView <= 1.0f)
	{
		Camera::fieldOfView = 1.0f;
	}

	// If the field of view of the camera is zoomed out by 45.0 or more, set the field of view equal to 45.0
	else if (Camera::fieldOfView >= 45.0f)
	{
		Camera::fieldOfView = 45.0f;
	}
}

void Window::ProcessInput(GLFWwindow* window)
{
	// Basically if we press the escape key while the window is running, it'll close the GLFW window for us
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);

		color->~Color();
		lighting->~Lighting();
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
		// Set the enum variable equal to move camera forward
		cameraMovement = CameraMovement::MoveCameraForward;

		// If the enum variable is definitely equal to move camera forward, which I already set to equal to it
		if (cameraMovement == CameraMovement::MoveCameraForward)
		{
			// Move the camera towards the screen when W is pressed
			Camera::cameraPosition += cameraMoveSpeed * Camera::cameraFront;
		}

		// I managed to get the up y-axis movement working here but I commented it out for the sake of the tutorial
		//Camera::cameraPosition += cameraMoveSpeed * glm::normalize(glm::cross(Camera::cameraRight, Camera::cameraFront));
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		// Set the enum variable equal to move camera backward
		cameraMovement = CameraMovement::MoveCameraBackward;

		// If the enum variable is definitely equal to move camera backward, which I already set to equal to it
		if (cameraMovement == CameraMovement::MoveCameraBackward)
		{
			// Move the camera away from the screen when S key is pressed
			Camera::cameraPosition -= cameraMoveSpeed * Camera::cameraFront;
		}
		
		// I managed to get the down y-axis movement working here but I commented it out for the sake of the tutorial
		//Camera::cameraPosition -= cameraMoveSpeed * glm::normalize(glm::cross(Camera::cameraRight, Camera::cameraFront));
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		// Set the enum variable equal to move camera left
		cameraMovement = CameraMovement::MoveCameraLeft;

		// If the enum variable is definitely equal to move camera left, which I already set to equal to it
		if (cameraMovement == CameraMovement::MoveCameraLeft)
		{
			// Move the camera left when A key is pressed
			Camera::cameraPosition -= glm::normalize(glm::cross(Camera::cameraFront, Camera::cameraUp)) * cameraMoveSpeed;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		// Set the enum variable equal to move camera right
		cameraMovement = CameraMovement::MoveCameraRight;

		// If the enum variable is definitely equal to move camera right, which I already set to equal to it
		if (cameraMovement == CameraMovement::MoveCameraRight)
		{
			// Move the camera right when D key is pressed
			Camera::cameraPosition += glm::normalize(glm::cross(Camera::cameraFront, Camera::cameraUp)) * cameraMoveSpeed;
		}
	}

	//Camera::cameraPosition.y = 0.0f; // Prevents flying or landing, staying at ground level (xz plane)
}