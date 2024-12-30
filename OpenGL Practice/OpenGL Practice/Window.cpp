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
		new VertexShaderLoader("LightCubeVertexShader.glsl"),
		new VertexShaderLoader("ModelVertexShader.glsl"),
		new VertexShaderLoader("DepthTestVertexShader.glsl"),
		new VertexShaderLoader("ColorVertexShader.glsl"),
		new VertexShaderLoader("TextureVertexShader.glsl"),
		new VertexShaderLoader("InversionVertexShader.glsl"),
		new VertexShaderLoader("GreyscaleVertexShader.glsl"),
		new VertexShaderLoader("KernelVertexShader.glsl"),
		new VertexShaderLoader("SkyboxVertexShader.glsl"),
		new VertexShaderLoader("EnvironmentMappingVertexShader.glsl"),
		new VertexShaderLoader("AdvancedGLSLVertexShader.glsl")
	};

	fragmentShaderLoader =
	{
		new FragmentShaderLoader("LightingFragmentShader.glsl"),
		new FragmentShaderLoader("LightCubeFragmentShader.glsl"),
		new FragmentShaderLoader("ModelFragmentShader.glsl"),
		new FragmentShaderLoader("DepthTestFragmentShader.glsl"),
		new FragmentShaderLoader("ColorFragmentShader.glsl"),
		new FragmentShaderLoader("TextureFragmentShader.glsl"),
		new FragmentShaderLoader("InversionFragmentShader.glsl"),
		new FragmentShaderLoader("GreyscaleFragmentShader.glsl"),
		new FragmentShaderLoader("KernelFragmentShader.glsl"),
		new FragmentShaderLoader("SkyboxFragmentShader.glsl"),
		new FragmentShaderLoader("EnvironmentMappingFragmentShader.glsl"),
		new FragmentShaderLoader("RedFragmentShader.glsl"),
		new FragmentShaderLoader("GreenFragmentShader.glsl"),
		new FragmentShaderLoader("BlueFragmentShader.glsl"),
		new FragmentShaderLoader("YellowFragmentShader.glsl")
	};

	openGLwindow = NULL;

	//shaderProgram = new ShaderProgram();
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

	model = nullptr;

	blendTexture = new Blending();
	faceCulling = new FaceCulling();
	framebuffer = new FrameBuffer();
	skybox = new Skybox();
	advancedData = new AdvancedData();

	geometryShader = new GeometryShader("GeometryShader.glsl");

	geometryShaderProgram = new ShaderProgram();

	instancing = new Instancing();

	antiAliasing = new AntiAliasing();
}

void Window::InitializeOpenGLwindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* GLFW also gives us this functionality and all we need to do is hint GLFW that we’d like to use a multisample buffer 
	with N samples instead of a normal buffer by calling glfwWindowHint before creating the window */
	
	// Buffer containing 4 subsamples per screen coordinate (Anti-aliasing Part 1)
	glfwWindowHint(GLFW_SAMPLES, 4);

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
	//shaderProgram->InitializeShaderProgram(vertexShaderLoader[2], fragmentShaderLoader[2]);
	//model = new Model("Models/Backpack/backpack.obj");

	//glEnable(GL_DEPTH_TEST);

	//skybox->SetCubeObject();
	//skybox->SetSkyboxObject();
	//skybox->SetCubeTexture();
	//skybox->SetSkyboxTexture();
	//advancedData->InitializeCubeVertices();
	//advancedData->InitializeBufferObject();
	//geometryShader->InitializeGeometryVertices(); // Geometry shader Part 1
	//geometryShader->InitializeGeometryModel(); // Geometry shader Part 2
	//instancing->SetInstancingOffsetPositions(); // Instancing Part 1
	//instancing->InitializeInstancingVertices(); // Instancing Part 1
	//instancing->SetTransformationMatrix(); // Instancing Part 2 & 3
	//instancing->SetInstancedArrays(); // Instancing Part 3 (for instanced rendering)

	antiAliasing->InitializeAntiAliasing(); // Anti-Aliasing Part 1

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
		//glEnable(GL_DEPTH_TEST);

		/* OpenGL also makes it possible to change the comparison operators it will use for depth testing. This means
		we can control when OpenGL should pass or discard fragments and when to update the depth buffer */

		/* By default, the depth function "GL_LESS" is used that discards all the fragments that have a depth value
		higher than or equal to the current depth buffer's value */
		//glDepthFunc(GL_LESS);

		/* We can discard certain fragments of other drawn objects in the scene by using the stencil buffer. By 
		enabling stencil testing, all rendering calls will influence the stencil buffer one way or another */
		/*glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);*/

		// Add our own color to the window
		//glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

		// Give a window a desert look (I got the colors using Adobe Color Wheel generator website)
		//glClearColor(0.91372549019f, 0.72549019607f, 0.38823529411f, 0.91f);

		// Make the window black (factory and horror)
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		// Make the window white (biochemical lab)
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		// If I get rid of this, my window will be black because then we didn't clear any color buffer bit first before rendering
		// Since we're also using depth buffer, we need to clear the depth buffer before each render iteration
		// Otherwise, the depth information of the previous frame stays in the buffer (and the cube won't render at all)
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		/* OpenGL can make it possible to NOT write to the depth buffer by setting OpenGL's built in
		depth mask to false using GL_FALSE, obviously (this only works when the depth testing in enabled) */
		//glDepthMask(GL_FALSE);

		/* glDepthFunc accepts several comparison operators like:
		
			GL_ALWAYS - The depth test always passes
			GL_NEVER - The depth test never passes
			GL_LESS - Passes if the fragment's depth value is less than the stored depth value
			GL_EQUAL - Passes if the fragment's depth value is equal to the stored depth value
			GL_LEQUAL - Passes if the fragemnt's depth value is less than or equal to the stored depth value
			GL_GREATER - Passes if the fragment's depth value is greater than the stored depth value
			GL_NOTEQUAL - Passes if the fragment's depth value is not equal to the stored depth value
			GL_GEQUAL - Passes if the fragment's depth value is greater than or equal to the stored depth value

		*/

		/* glStencilMask allows us to set a bitmask that is ANDed with the stencil value about to be written to the
		buffer. By default this is set to a bitmask of all 1s not affecting the output, but if we set it to 0x00
		all the stencil values written to the buffer end up as 0s */
		//glStencilMask(0xFF);
		//glStencilMask(0x00);

		/* glStencilFunc passes in 3 parameters:
			
			func - sets the stencil test function to determine whether the fragment passes or is discarded. The test
			applies to the stored stencil value and the glStencilFunc's ref value. Possible options are similar to
			the depth buffer's func.

			ref - specifies the reference value for the stencil test. The stencil's buffer content is compared to this
			value

			mask - specifies a mask that is ANDed with both reference value and the stored stencil value before the
			test compares them, which initializes to be 1 by default

		*/

		/* Whenever the stencil value of a fragment is equal to 1, the fragment passes the test and is drawn, or else
		it will be discarded */
		//glStencilFunc(GL_EQUAL, 1, 0xFF);

		/* glStencilOp passes in 3 parameters:
			
			sfail - if the stencil test fails, what action should it take?
			defail - if the stencil test passes but the stencil test fails, what action should it take?
			depass - if both the stencil and depth tests pass, what action should it take?

		*/

		/* For each of the glStencilOp parameters, we can pick any action we want:
			
			GL_KEEP - the currently stored stencil value is kept
			GL_ZERO - the stencil value is set to 0
			GL_REPLACE - the stencil value is replaced with the reference value set with glStencilFunc
			GL_INCR - the stencil value is increased by 1 if it's lower than maximum value
			GL_INCR_WRAP - Same as GL_INCR, except wraps it back to 0 as soon as the maximum value is exceeded
			GL_DECR - the stencil value is decreased by 1 if it's higher than maximum value
			GL_DECR_WRAP - Same as GL_DECR, except wraps it back to the maximum value if it ends up lower than 0
			GL_INVERT - Bitwise inverts the current stencil buffer value

		*/

		//shaderProgram->InitializeShaderProgram(vertexShaderLoader[0], fragmentShaderLoader[0]);
		//vertexShaderLoader[0]->InitializeVertexObjects();

		// Call the 3D object initialization function to render 3D objects onto the OpenGL window
		// The 800.0f/ 600.0f below also equals to 4.0f / 3.0f which is what most graphics aspect ratios are defaulted to
		//shaderProgram->InitializeCubeColor(800.0f / 600.0f, 0.1f, 100.0f);

		//shaderProgram->InitializeShaderProgram(vertexShaderLoader[1], fragmentShaderLoader[1]);
		//vertexShaderLoader[1]->InitializeVertexObjects();

		// Call the second texture initialization function here to render the second container and scale it overtime
		//shaderProgram->InitializeSecondTexture();

		//shaderProgram->InitializeLightColor(800.0f / 600.0f, 0.1f, 100.0f);

		// To render images with different levels of transparency, enable OpenGL blending
		//glEnable(GL_BLEND);

		// The glBlendFunc function passes in 2 parameters that set the option for the source and destination factor

		/* OpenGL has multiple blending options including:
			
			GL_ZERO - Factor is equal to 0
			GL_ONE - Factor is equal to 1
			GL_SRC_COLOR - Factor is equal to the source color vector (Cs)
			GL_ONE_MINUS_SRC_COLOR - Factor is equal to 1 minus the source color vector (1 - Cs)
			GL_DST_COLOR - Factor is equal to the destination color vector (Cd)
			GL_ONE_MINUS_DST_COLOR - Factor is equal to 1 minus the destination color vector (1 - Cd)
			GL_SRC_ALPHA - Factor is equal to the alpha component of the source color vector (Cs)
			GL_ONE_MINUS_SRC_ALPHA - Factor is equal to 1 minus alpha of the source color vector (1 - Cs)
			GL_DST_ALPHA - Factor is equal to the alpha component of the destination color vector (Cd)
			GL_ONE_MINUS_DST_ALPHA - Factor is equal to 1 minus alpha of the destination color vector (1 - Cd)
			GL_CONSTANT_COLOR - Factor is equal to the constant color vector (Cc)
			GL_ONE_MINUS_CONSTANT_COLOR - Factor is equal to 1 minus the constant color vector (1 - Cc)
			GL_CONSTANT_ALPHA - Factor is equal to the alpha component of the constant color vector (Cca)
			GL_ONE_MINUS_CONSTANT_ALPHA - Factor is equal to 1 minus alpha of the constant color vector (1 - Cca)

		*/

		// Take the alpha component of the source color vector for the source factor
		// Then take 1 - alpha of the same color (source) vector for the destination factor
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		/* This function sets the RGB components, as I set it above but only lets the resulting alpha component be
		influenced by the source's alpha value */
		//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		/* glBlendEquation(GLenum mode) has 5 possible options that can be passed in inside the GLenum parameter:
			
			GL_FUNC_ADD - the default, adds both colors to each other (Cr = Src + Dst)
			GL_FUNC_SUBTRACT - subtracts both colors from each other (Cr = Src - Dst)
			GL_FUNC_REVERSE_SUBTRACT - subtracts both colors, but reverses order (Cr = Dst - Src)
			GL_MIN - takes the component-wise minimum of both colors (Cr = min(Dst, Src))
			GL_MAX - takes the component-wise maximum of both colors (Cr = max(Dst, Src))

		*/
		
		//IncludeFrameBufferMethods();
		//UseSkybox();

		// Use the depth testing shader
		//shaderProgram->InitializeShaderProgram(vertexShaderLoader[3], fragmentShaderLoader[3]);

		//glStencilFunc(GL_ALWAYS, 1, 0xFF);
		//glStencilMask(0xFF);

		//vertexShaderLoader[3]->InitializeCubeDepthTestingVertices();
		//faceCulling->SetFaceCullingVertices();

		//framebuffer->AddFrameBuffer();
		//shaderProgram->InitializeCubeDepthTesting(800.0f / 600.0f, 0.1f, 100.0f);

		//glStencilMask(0x00);

		//vertexShaderLoader[3]->InitializeFloorDepthTestingVertices();
		//shaderProgram->InitializeFloorDepthTesting();

		//vertexShaderLoader[3]->InitializeFloorDepthTestingVertices();

		// Use the grass blend texture here
		/*blendTexture->SetBlending();
		blendTexture->IncludeGrassBlending();*/

		// Use the transparent window blend texture here
		//blendTexture->SetBlending();
		//blendTexture->IncludeTransparentWindowBlending();

		// Now we can use the border color shader
		/*shaderProgram->InitializeShaderProgram(vertexShaderLoader[4], fragmentShaderLoader[4]);
		glUseProgram(ShaderProgram::shaderProgram);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		// Use the border color vertex shader to initialize the cube's vertices
		vertexShaderLoader[4]->InitializeCubeDepthTestingVertices();

		// Then use the border color shader to render the border color on the scaled cube
		shaderProgram->InitializeScaledCubeStencilTesting(800.0f / 600.0f, 0.1f, 100.0f);
		model->DrawModel();
		
		AdvancedGLSL();
		UseGeometryShader();
		UseInstancingClass();*/
		
		UseAntiAliasing();

		glfwSwapBuffers(openGLwindow); // Removing this will throw an exception error
		glfwPollEvents(); // Waits for any input by the user and processes it in real-time
	}

	/*for (int i = 0; i < vertexShaderLoader.size(); i++)
	{
		vertexShaderLoader[i]->~VertexShaderLoader();
	}*/

	//blendTexture->~Blending();
	//faceCulling->~FaceCulling();
	//framebuffer->~FrameBuffer();
	//skybox->~Skybox();
	//advancedData->~AdvancedData();
	//geometryShader->~GeometryShader();
	//instancing->~Instancing();

	antiAliasing->~AntiAliasing();

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
	if (Camera::constrainPitch == true)
	{
		if (Camera::pitch >= 89.0f)
		{
			Camera::pitch = 89.0f;
		}

		else if (Camera::pitch <= -89.0f)
		{
			Camera::pitch = -89.0f;
		}
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
	/*if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
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
	}*/

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

void Window::UseAntiAliasing()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	antiAliasing->RenderAntiAliasing();
}

/*void Window::UseInstancingClass()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	instancing->UseInstancingShaderProgram();
}*/

/*void Window::UseGeometryShader()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//geometryShader->DrawExplodingModel(); // Geometry Shader part 2

	geometryShader->DrawVisualizingNormalVectors(); // Geometry Shader part 3
}*/

/*void Window::AdvancedGLSL()
{
	// For enabling the influence of point sizes in the vertex shader
	//glEnable(GL_PROGRAM_POINT_SIZE);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	advancedData->LoadCubesInfo();

	advancedData->DrawRedCube();
	advancedData->DrawGreenCube();
	advancedData->DrawBlueCube();
	advancedData->DrawYellowCube();
}*/

// I created a function to store all the frame buffer methods in the FrameBuffer class and execute it during runtime
/*void Window::IncludeFrameBufferMethods()
{
	// Use the texture shaders
	shaderProgram->InitializeShaderProgram(vertexShaderLoader[5], fragmentShaderLoader[5]);

	// Use the inversion texture shaders
	//shaderProgram->InitializeShaderProgram(vertexShaderLoader[6], fragmentShaderLoader[6]);

	// Use the greyscale texturing shaders
	//shaderProgram->InitializeShaderProgram(vertexShaderLoader[7], fragmentShaderLoader[7]);

	// Use the kernel texturing shaders
	//shaderProgram->InitializeShaderProgram(vertexShaderLoader[8], fragmentShaderLoader[8]);

	framebuffer->InitializeCubeVertices();
	framebuffer->InitializePlaneVertices();
	framebuffer->InitializeQuadVertices();

	framebuffer->AddFrameBuffer(); // needs to be commented for inversion texturing to work

	framebuffer->RenderScene();

	// Good thing I have a static variable of my very own shader program variable
	glUseProgram(ShaderProgram::shaderProgram);

	framebuffer->InitializeCubeTextures();
	shaderProgram->InitializeCubeDepthTesting(800.0f / 600.0f, 0.1f, 100.0f);

	framebuffer->InitializePlaneTextures();
	shaderProgram->InitializeFloorDepthTesting();

	framebuffer->BindToDefaultFrameBuffer(); // needs to be commented for inversion texturing to work
	framebuffer->InitializeQuadTextures(); // needs to be commented to show how inversion textures work
} */

/*void Window::UseSkybox()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the texture shader files
	shaderProgram->InitializeShaderProgram(vertexShaderLoader[10], fragmentShaderLoader[10]);

	skybox->UseShaderProgramForCube(800.0f / 600.0f, 0.1f, 100.0f);

	glDepthFunc(GL_LEQUAL);

	// Use the skybox shader files for me
	shaderProgram->InitializeShaderProgram(vertexShaderLoader[9], fragmentShaderLoader[9]);

	skybox->UseShaderProgramForSkybox(800.0f / 600.0f, 0.1f, 100.0f);

	glDepthFunc(GL_LESS);
}*/
