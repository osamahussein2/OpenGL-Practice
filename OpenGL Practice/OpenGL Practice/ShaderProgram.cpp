#include "ShaderProgram.h"

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