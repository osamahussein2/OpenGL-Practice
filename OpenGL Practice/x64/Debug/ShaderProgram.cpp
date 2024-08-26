#include "ShaderProgram.h"
#include "Window.h"

ShaderProgram::ShaderProgram()
{
	vertexShaderLoader = new VertexShaderLoader("VertexShader.glsl");
	fragmentShaderLoader = new FragmentShaderLoader("FragmentShader.glsl");

	timer = 0.0f;
	moveRight = 0.0f;
	movingPositionLocation = 0;

	shaderProgram = NULL;

	visibilityTextureValue = 0.2f;

	transformMatrixLocation = NULL;

	vector = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	translateMatrix = glm::mat4(0.0f);

	modelMatrix = glm::mat4(0.0f);
	viewMatrix = glm::mat4(0.0f);
	projectionMatrix = glm::mat4(0.0f);

	modelMatrixLocation = NULL;
	viewMatrixLocation = NULL;
	projectionMatrixLocation = NULL;

	for (int cubePositionsArray = 0; cubePositionsArray < cubePositions.size(); cubePositionsArray++)
	{
		// Initialize the vec3 array elements to 0 for all axes
		cubePositions[cubePositionsArray] = glm::vec3(0.0f, 0.0f, 0.0f);
	}
}

ShaderProgram::~ShaderProgram()
{
	// Delete the vertex and fragment shaders after the shader objects have been linked with the shader program
	glDeleteShader(vertexShaderLoader->vertexShader);
	glDeleteShader(fragmentShaderLoader->fragmentShader);
}

void ShaderProgram::InitializeShaderProgram()
{
	// Create a shader program that will render both the vertex and fragment shaders to the window
	shaderProgram = glCreateProgram();

	vertexShaderLoader->InitializeVertexShaderLoader();
	fragmentShaderLoader->InitializeFragmentShaderLoader();

	// Attach the vertex and fragment shaders to our created shader program ID object
	glAttachShader(shaderProgram, vertexShaderLoader->vertexShader);
	glAttachShader(shaderProgram, fragmentShaderLoader->fragmentShader);

	// Link the attached vertex and fragment shaders together into one shader program
	glLinkProgram(shaderProgram);

	int successfullyCompiled; // An integer that checks if the shader program compilation was successful
	char compilationInformationLog[512]; // Gives information about its compilation in a log file (maybe?)

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &successfullyCompiled);

	if (!successfullyCompiled)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, compilationInformationLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << compilationInformationLog << std::endl;
	}

	timer = glfwGetTime(); // Gets the time in seconds using the GLFW library
	moveRight = ((timer) / 5.0f) + 0.1f;

	// Query the location of the uniform color in the fragment shader
	// Pass in the shader program and the uniform color's name inside the glGetUniformLocation defined parameters
	movingPositionLocation = glGetUniformLocation(shaderProgram, "movingPosition");

	// Activate the shader program using glUseProgram function to use it
	glUseProgram(shaderProgram); // This helps set the uniform on the currently running shader program (updating it)

	// Set the uniform value
	glUniform3f(movingPositionLocation, moveRight, 0.0f, 0.0f);

	glUniform1i(glGetUniformLocation(shaderProgram, "theFirstTexture"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "theSecondTexture"), 1);
	glUniform1f(glGetUniformLocation(shaderProgram, "visibilityTexture"), visibilityTextureValue);

	// Initialize the vector by declaring it as a vec4 using the GLM library
	//vector = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	// Use mat4 for 4D matrix and set it to initialize the identity matrix by initializing the matrix's diagonals to 1

	/* If this is not initialized to its identity matrix, the matrix will be a null matrix(all elements equal to 0) and
	all subsequent matrix operations would be null as well (we don't want that, could lead to unexpected problems) */
	translateMatrix = glm::mat4(1.0f);

	/* Since I rotated the matrix first, it will rotate along the z axis per frame and then translate it to the bottom
	right of the window each time the rotation loop is occuring, meaning that the image will rotate around the screen. 
	It kind of looks like the container texture is swinging a pendulum. */

	/* Whereas if I rotated the matrix after translating, it will immediately place the texture at the bottom right of
	the screen and then rotate it without moving the image around the window because of the fact that the translation
	of the texture has already finished placing it at the bottom right of the window. */

	// Rotate the matrix on each iteration of the render loop by using glfwGetTime()
	translateMatrix = glm::rotate(translateMatrix, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

	// Translate the matrix at the bottom right of the OpenGL window (or move this at bottom right of the window)
	translateMatrix = glm::translate(translateMatrix, glm::vec3(0.5f, -0.5f, 0.0f));

	//translateMatrix = glm::translate(translateMatrix, glm::vec3(1.0f, 1.0f, 0.0f));

	// Rotate the container texture 90 degrees around the z-axis (there is 1.0f in z axis below)
	//translateMatrix = glm::rotate(translateMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// Scale the container texture by half of its normal scale on each axis
	//translateMatrix = glm::scale(translateMatrix, glm::vec3(0.5f, 0.5f, 0.5f));

	//vector = translateMatrix * vector;
	//std::cout << "This vector matrix is equal to (" << vector.x << ", " << vector.y << ", " << vector.z << ")" << "\n";

	transformMatrixLocation = glGetUniformLocation(shaderProgram, "translateMat");
	glUniformMatrix4fv(transformMatrixLocation, 1, GL_FALSE, glm::value_ptr(translateMatrix));
}

// Render the second container texture in the window
void ShaderProgram::InitializeSecondTexture()
{
	translateMatrix = glm::mat4(1.0f);

	// Rotate the matrix on each iteration of the render loop by using glfwGetTime()
	translateMatrix = glm::scale(translateMatrix,
		glm::vec3(sin((float)glfwGetTime()), sin((float)glfwGetTime()), sin((float)glfwGetTime())));

	// Translate the matrix at the bottom right of the OpenGL window (or move this at bottom right of the window)
	translateMatrix = glm::translate(translateMatrix, glm::vec3(-0.5f, 0.5f, 0.0f));

	glUniformMatrix4fv(transformMatrixLocation, 1, GL_FALSE, &translateMatrix[0][0]);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void ShaderProgram::Initialize3Dobjects(float aspect_ratio, float near_plane, float far_plane)
{
	// Initialize the translation vector array for each cube that specifies its position in world space
	cubePositions = {
		glm::vec3(0.0f, 0.0f, -3.0f),
		glm::vec3(2.0f, 5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f, 3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f, 2.0f, -2.5f),
		glm::vec3(1.5f, 0.2f, -1.5f),
		glm::vec3(-1.3f, 1.0f, -1.5f)
	};

	/* After the model matrix has been created and set up, we need to create a view matrix. We have to move
	slightly backwards in the scene so that the objects are visible inside the OpenGL window (when we're in world 
	space, we are located at the origin (or in other words, the coordinate (0, 0, 0). */

	/* The view matrix basically moves the entire scene around inversed (or reversed) to where we want the camera to 
	move. Because we want to move backwards and since OpenGL is a right-handed system we have to move in the positive 
	z-axis. This gives the impression that we're moving backwards. */
	viewMatrix = glm::mat4(1.0f);
	//viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.0f, -3.0f)); // Translate the scene in the reversed direction

	/* The LookAt matrix defines a coordinate space using 3 perpendicular (or non-linear) axes and this is where
	you can create a matrix with those 3 axes plus a translation vector, that way you can transform any vector to
	to that coordinate space simply by multiplying it with the LookAt matrix. */

	// LookAt function passes in 3 parameters, one for camera's position, two for target, and three for up vector
	//viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Call the initialize camera function before setting the view matrix or else the cubes won't render on the window
	//camera->InitializeCamera();

	/*viewMatrix = glm::lookAt(glm::vec3(camera->cameraX, 0.0, camera->cameraZ), glm::vec3(0.0, 0.0, 0.0),
	glm::vec3(0.0, 1.0, 0.0)); */

	// First we must set the camera position to the camera position vec3 we defined in the camera class

	/* Second we must set the direction to add the camera's current position and the camera's direction vector. 
	Basically, this will make sure however we move, the camera will keep looking at the target direction. */
	viewMatrix = glm::lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);

	viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	/* The last thing we need to define is the projection matrix where we're going to use it in our scene */

	/* The glm::perspective function passes in 4 float values: the first one passes in the field of view in the y
	direction, the second one passes in the aspect ratio value, the third one passes in the near plane value (or the
	distance from the viewer to the near clipping plane point (always positive, so most likely we need an unsigned int 
	reference here), and the fourth one passes in the far plane value (or the distance from the viewer to the far
	clipping plane point (again, always positive so another potential unsigned int is mandatory here) */
	projectionMatrix = glm::perspective(glm::radians(45.0f), aspect_ratio, near_plane, far_plane);

	projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Loop through all the cubes positions array
	for (unsigned int renderCubes = 0; renderCubes < cubePositions.size(); renderCubes++)
	{
		// To be able to draw in 3D, we will need a model matrix

		/* The model matrix includes translations, scaling and /or rotations we want to make to transform all the object's
		vertices to the global world space */

		/* By multiplying the vertex coordinates with the model matrix, we are essentially converting the vertex
		coordinates from local space to the world space. That means whatever we're doing using the model matrix, we
		are forcing the vertex coordinates to represent the plane in world space rather than the space of itself. */
		modelMatrix = glm::mat4(1.0f);

		// Use the model matrix to place all the cube positions elements somewhere inside the window
		modelMatrix = glm::translate(modelMatrix, cubePositions[renderCubes]);

		// Create an angle variable to determine the angle that each cube should rotate at
		float angle = 20.0f * renderCubes;

		// Rotate each container by the angle they're equal to
		modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

		// Set the model matrix location to look for the modelMatrix uniform set inside the vertex shader GLSL file
		modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");

		// Set the uniform model matrix location to equal to the modelMatrix value pointer
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		/* Make sure to draw arrays at the end of setting the model, view and projection matrices locations to render
		the cube on the window */
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}