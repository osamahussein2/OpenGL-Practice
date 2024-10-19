#include "ShaderProgram.h"
#include "Window.h"

unsigned int ShaderProgram::shaderProgram = NULL;

ShaderProgram::ShaderProgram()
{
	vertexShaderLoader = { 
		new VertexShaderLoader("LightingVertexShader.glsl"), 
		new VertexShaderLoader("LightCubeVertexShader.glsl"),
		new VertexShaderLoader("ModelVertexShader.glsl"),
		new VertexShaderLoader("DepthTestVertexShader.glsl"),
		new VertexShaderLoader("ColorVertexShader.glsl")
	};

	fragmentShaderLoader = { 
		new FragmentShaderLoader("LightingFragmentShader.glsl"), 
		new FragmentShaderLoader("LightCubeFragmentShader.glsl"),
		new FragmentShaderLoader("ModelFragmentShader.glsl"),
		new FragmentShaderLoader("DepthTestFragmentShader.glsl"),
		new FragmentShaderLoader("ColorFragmentShader.glsl")
	};

	color = new Color();
	lighting = new Lighting();

	timer = 0.0f;
	moveRight = 0.0f;
	movingPositionLocation = 0;

	visibilityTextureValue = 0.2f;

	transformMatrixLocation = NULL;

	objectColorLocation = NULL;
	lightColorLocation = NULL;

	lightPositionLocation = NULL;
	viewPositionLocation = NULL;

	vector = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	translateMatrix = glm::mat4(0.0f);

	modelMatrix = glm::mat4(0.0f);
	viewMatrix = glm::mat4(0.0f);
	projectionMatrix = glm::mat4(0.0f);

	modelMatrixLocation = NULL;
	viewMatrixLocation = NULL;
	projectionMatrixLocation = NULL;

	lightPosition = glm::vec3(0.0f, 0.0f, 0.0f);

		// Initialize the vec3 array elements to 0 for all axes
	cubePositions = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f) };

	pointLightPositions = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f)};
}

ShaderProgram::~ShaderProgram()
{
	// Delete the vertex and fragment shaders after the shader objects have been linked with the shader program
	glDeleteShader(vertexShaderLoader[0]->vertexShader);
	glDeleteShader(fragmentShaderLoader[0]->fragmentShader);

	glDeleteShader(vertexShaderLoader[1]->vertexShader);
	glDeleteShader(fragmentShaderLoader[1]->fragmentShader);

	glDeleteShader(vertexShaderLoader[2]->vertexShader);
	glDeleteShader(fragmentShaderLoader[2]->fragmentShader);

	glDeleteShader(vertexShaderLoader[3]->vertexShader);
	glDeleteShader(fragmentShaderLoader[3]->fragmentShader);

	glDeleteShader(vertexShaderLoader[4]->vertexShader);
	glDeleteShader(fragmentShaderLoader[4]->fragmentShader);
}

void ShaderProgram::InitializeShaderProgram(VertexShaderLoader* vertexShader_, FragmentShaderLoader* fragmentShader_)
{
	// Create a shader program that will render both the vertex and fragment shaders to the window
	shaderProgram = glCreateProgram();

	vertexShader_->InitializeVertexShaderLoader();
	fragmentShader_->InitializeFragmentShaderLoader();

	// Attach the vertex and fragment shaders to our created shader program ID object
	glAttachShader(shaderProgram, vertexShader_->vertexShader);
	glAttachShader(shaderProgram, fragmentShader_->fragmentShader);

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

	/*timer = glfwGetTime(); // Gets the time in seconds using the GLFW library
	moveRight = ((timer) / 5.0f) + 0.1f;

	// Query the location of the uniform color in the fragment shader
	// Pass in the shader program and the uniform color's name inside the glGetUniformLocation defined parameters
	movingPositionLocation = glGetUniformLocation(shaderProgram, "movingPosition");*/

	// Activate the shader program using glUseProgram function to use it
	//glUseProgram(shaderProgram); // This helps set the uniform on the currently running shader program (updating it)

	// Set the uniform value
	/*glUniform3f(movingPositionLocation, moveRight, 0.0f, 0.0f);

	glUniform1i(glGetUniformLocation(shaderProgram, "theFirstTexture"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "theSecondTexture"), 1);
	glUniform1f(glGetUniformLocation(shaderProgram, "visibilityTexture"), visibilityTextureValue); */

	// Initialize the vector by declaring it as a vec4 using the GLM library
	//vector = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	// Use mat4 for 4D matrix and set it to initialize the identity matrix by initializing the matrix's diagonals to 1

	/* If this is not initialized to its identity matrix, the matrix will be a null matrix(all elements equal to 0) and
	all subsequent matrix operations would be null as well (we don't want that, could lead to unexpected problems) */
	//translateMatrix = glm::mat4(1.0f);

	/* Since I rotated the matrix first, it will rotate along the z axis per frame and then translate it to the bottom
	right of the window each time the rotation loop is occuring, meaning that the image will rotate around the screen.
	It kind of looks like the container texture is swinging a pendulum. */

	/* Whereas if I rotated the matrix after translating, it will immediately place the texture at the bottom right of
	the screen and then rotate it without moving the image around the window because of the fact that the translation
	of the texture has already finished placing it at the bottom right of the window. */

	// Rotate the matrix on each iteration of the render loop by using glfwGetTime()
	//translateMatrix = glm::rotate(translateMatrix, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

	// Translate the matrix at the bottom right of the OpenGL window (or move this at bottom right of the window)
	//translateMatrix = glm::translate(translateMatrix, glm::vec3(0.5f, -0.5f, 0.0f));

	//translateMatrix = glm::translate(translateMatrix, glm::vec3(1.0f, 1.0f, 0.0f));

	// Rotate the container texture 90 degrees around the z-axis (there is 1.0f in z axis below)
	//translateMatrix = glm::rotate(translateMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// Scale the container texture by half of its normal scale on each axis
	//translateMatrix = glm::scale(translateMatrix, glm::vec3(0.5f, 0.5f, 0.5f));

	//vector = translateMatrix * vector;
	//std::cout << "This vector matrix is equal to (" << vector.x << ", " << vector.y << ", " << vector.z << ")" << "\n";

	/*transformMatrixLocation = glGetUniformLocation(shaderProgram, "translateMat");
	glUniformMatrix4fv(transformMatrixLocation, 1, GL_FALSE, glm::value_ptr(translateMatrix));*/
}

void ShaderProgram::InitializeModeling(float aspect_ratio, float near_plane, float far_plane)
{
	projectionMatrix = glm::perspective(glm::radians(Camera::fieldOfView), aspect_ratio, near_plane, far_plane);

	projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	viewMatrix = Camera::CameraLookAt();

	viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

	modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");

	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
}

void ShaderProgram::InitializeCubeDepthTesting(float aspect_ratio, float near_plane, float far_plane)
{
	// Set the texture image uniform
	glUniform1i(glGetUniformLocation(shaderProgram, "textureImage"), 0);
	
	glUniform1f(glGetUniformLocation(shaderProgram, "near"), near_plane);
	glUniform1f(glGetUniformLocation(shaderProgram, "far"), far_plane);

	projectionMatrix = glm::perspective(glm::radians(Camera::fieldOfView), aspect_ratio, near_plane, far_plane);

	projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	viewMatrix = Camera::CameraLookAt();

	viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, 0.0f, -1.0f));

	modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");

	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	glDrawArrays(GL_TRIANGLES, 0, 36);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f, 0.0f, 0.0f));

	modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");

	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void ShaderProgram::InitializeScaledCubeStencilTesting(float aspect_ratio, float near_plane, float far_plane)
{
	glUniform1i(glGetUniformLocation(shaderProgram, "textureImage"), 0);

	glUniform1f(glGetUniformLocation(shaderProgram, "near"), near_plane);
	glUniform1f(glGetUniformLocation(shaderProgram, "far"), far_plane);

	projectionMatrix = glm::perspective(glm::radians(Camera::fieldOfView), aspect_ratio, near_plane, far_plane);

	projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	viewMatrix = Camera::CameraLookAt();

	viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	glUniform1f(glGetUniformLocation(shaderProgram, "redValue"), 0.04f);
	glUniform1f(glGetUniformLocation(shaderProgram, "greenValue"), 0.28f);
	glUniform1f(glGetUniformLocation(shaderProgram, "blueValue"), 0.26f);
	glUniform1f(glGetUniformLocation(shaderProgram, "alphaValue"), 1.0f);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, 0.0f, -1.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.1f, 1.1f, 1.1f));

	modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");

	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	glDrawArrays(GL_TRIANGLES, 0, 36);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f, 0.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.1f, 1.1f, 1.1f));

	modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");

	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void ShaderProgram::InitializeFloorDepthTesting()
{
	modelMatrix = glm::mat4(1.0f);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Render the second container texture in the window
//void ShaderProgram::InitializeSecondTexture()
//{
//	translateMatrix = glm::mat4(1.0f);
//
//	// Rotate the matrix on each iteration of the render loop by using glfwGetTime()
//	translateMatrix = glm::scale(translateMatrix,
//		glm::vec3(sin((float)glfwGetTime()), sin((float)glfwGetTime()), sin((float)glfwGetTime())));
//
//	// Translate the matrix at the bottom right of the OpenGL window (or move this at bottom right of the window)
//	translateMatrix = glm::translate(translateMatrix, glm::vec3(-0.5f, 0.5f, 0.0f));
//
//	glUniformMatrix4fv(transformMatrixLocation, 1, GL_FALSE, &translateMatrix[0][0]);
//
//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//}

//void ShaderProgram::InitializeCubeColor(float aspect_ratio, float near_plane, float far_plane)
//{
//	// Initialize the translation vector array for each cube that specifies its position in world space
//	cubePositions = {
//		glm::vec3(0.0f, 0.0f, 0.0f),
//		glm::vec3(2.0f, 5.0f, -15.0f),
//		glm::vec3(-1.5f, -2.2f, -2.5f),
//		glm::vec3(-3.8f, -2.0f, -12.3f),
//		glm::vec3(2.4f, -0.4f, -3.5f),
//		glm::vec3(-1.7f, 3.0f, -7.5f),
//		glm::vec3(1.3f, -2.0f, -2.5f),
//		glm::vec3(1.5f, 2.0f, -2.5f),
//		glm::vec3(1.5f, 0.2f, -1.5f),
//		glm::vec3(-1.3f, 1.0f, -1.5f)
//	};
//
//	pointLightPositions = {
//		glm::vec3(0.7f, 0.2f, 2.0f),
//		glm::vec3(2.3f, -3.3f, -4.0f),
//		glm::vec3(-4.0f, 2.0f, -12.0f),
//		glm::vec3(0.0f, 0.0f, -3.0f)
//	};
//
//	lightPosition = glm::vec3(1.2f, 1.0f, 2.0f);
//
//	// Move the light around the OpenGL window overtime
//	//lightPosition = glm::vec3(sin(glfwGetTime() * 1.2f), sin(glfwGetTime() * 1.0f), sin(glfwGetTime() * 2.0f));
//
//	color->InitializeColor();
//
//	objectColorLocation = glGetUniformLocation(shaderProgram, "objectColor");
//	glUniform3f(objectColorLocation, color->toyColor.x, color->toyColor.y, color->toyColor.z);
//
//	lightColorLocation = glGetUniformLocation(shaderProgram, "lightColor");
//	glUniform3f(lightColorLocation, 1.0f, 1.0f, 1.0f);
//
//	lightPositionLocation = glGetUniformLocation(shaderProgram, "light.positionalLight");
//	glUniform3fv(lightPositionLocation, 1, glm::value_ptr(Camera::cameraPosition));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.lightPosition"), 1,
//		glm::value_ptr(Camera::cameraPosition));
//
//	/*glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.lightDirection"), 1,
//		glm::value_ptr(lighting->SetDirectionalLighting(glm::vec3(-0.2f, -1.0f, -0.3f))));*/
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.directionalLight"), 1,
//		glm::value_ptr(lighting->SetDirectionalLighting(glm::vec3(Camera::cameraFront))));
//
//	viewPositionLocation = glGetUniformLocation(shaderProgram, "viewPosition");
//	glUniform3fv(viewPositionLocation, 1, glm::value_ptr(Camera::cameraPosition));
//
//	/* Let's set the uniform vector 3 to find a uniform type of material.ambientLight and set the vec3 values to 
//	the ambient lighting vector 3 that I set inside the Lighting class (go to Lighting.h and .cpp for more clarity)
//
//	// Set the ambient, diffuse and specular lighting materials to cyan plastic material
//	glUniform3fv(glGetUniformLocation(shaderProgram, "material.ambientLight"), 1,
//		glm::value_ptr(lighting->SetMaterialAmbientLighting(glm::vec3(0.2f, 0.2f, 0.2f))));*/
//
//	/* Let's set the uniform vector 3 to find a uniform type of material.diffuseLight and set the vec3 values to
//	the diffuse lighting vector 3 that I set inside the Lighting class
//	glUniform3fv(glGetUniformLocation(shaderProgram, "material.diffuseLight"), 1, 
//		glm::value_ptr(lighting->SetMaterialDiffuseLighting(glm::vec3(0.0f, 0.50980392f, 0.50980392f))));*/
//
//	/* Let's set the uniform vector 3 to find a uniform type of material.specularLight and set the vec3 values to
//	the specular lighting vector 3 that I set inside the Lighting class
//	glUniform3fv(glGetUniformLocation(shaderProgram, "material.specularLight"), 1,
//		glm::value_ptr(lighting->SetMaterialSpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f)))); */
//
//	// Let's set the uniform int to find a uniform type of material.diffuseMap and set it to 0
//	glUniform1i(glGetUniformLocation(shaderProgram, "material.diffuseMap"), 0);
//
//	// Let's set the uniform int to find a uniform type of material.specularMap and set it to 1
//	glUniform1i(glGetUniformLocation(shaderProgram, "material.specularMap"), 1);
//
//	// Let's set the uniform int to find a uniform type of material.emissionMap and set it to 2
//	//glUniform1i(glGetUniformLocation(shaderProgram, "material.emissionMap"), 2);
//
//	/* Let's set the uniform float to find a uniform type of material.shininess and set the float value to the shininess
//	value that I set inside the Lighting class */
//	glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), lighting->SetShininessLighting(32.0f));
//
//	//CreateDesertLighting();
//	//CreateFactoryLighting();
//	//CreateHorrorLighting();
//	//CreateBioChemicalLabLighting();
//
//	/* Let's set the uniform vector 3 to find a uniform type of light.ambientLight and set the vec3 values to
//	the ambient lighting intensity vector 3 that I set inside the Lighting class */
//
//	/* Set the ambient lighting intensity color to a lower value so that it doesn't have a lot of impact of the
//	surface's final color */
//	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.ambientLight"), 1,
//	glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));
//
//	/* Let's set the uniform vector 3 to find a uniform type of light.diffuseLight and set the vec3 values to
//	the diffuse lighting intensity vector 3 that I set inside the Lighting class */
//
//	/* Set the diffuse lighting intensity to be the light color we want */
//	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.diffuseLight"), 1,
//		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.4f, 0.4f, 0.4f))));
//
//	/* Let's set the uniform vector 3 to find a uniform type of light.specularLight and set the vec3 values to
//	the specular lighting intensity vector 3 that I set inside the Lighting class */
//
//	/* Set the specular lighting intensity to shine at full intensity */
//	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.specularLight"), 1,
//		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(0.5f, 0.5f, 0.5f))));
//
//	// Set the point light uniform positions array
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLights[0].lightPosition"), 1,
//		glm::value_ptr(pointLightPositions[0]));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLights[1].lightPosition"), 2,
//		glm::value_ptr(pointLightPositions[1]));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLights[2].lightPosition"), 3,
//		glm::value_ptr(pointLightPositions[2]));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLights[3].lightPosition"), 4,
//		glm::value_ptr(pointLightPositions[3]));
//
//	// Set the point lighting ambient, diffuse and specular light uniforms
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].ambientLight"), 1,
//		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].diffuseLight"), 1,
//		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.8f, 0.8f, 0.8f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].specularLight"), 1,
//		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].ambientLight"), 2,
//		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].diffuseLight"), 2,
//		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.8f, 0.8f, 0.8f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].specularLight"), 2,
//		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].ambientLight"), 3,
//		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].diffuseLight"), 3,
//		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.8f, 0.8f, 0.8f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].specularLight"), 3,
//		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].ambientLight"), 4,
//		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].diffuseLight"), 4,
//		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.8f, 0.8f, 0.8f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].specularLight"), 4,
//		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));
//
//	// Set the spot lighting diffuse and specular light uniforms
//	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.ambientLight"), 1,
//		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.0f, 0.0f, 0.0f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.diffuseLight"), 1,
//		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(1.0f, 1.0f, 1.0f))));
//
//	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.specularLight"), 1,
//		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));
//
//	// Try to cover the light distance of up to 50
//	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.constant"), lighting->SetAttenuationConstant(1.0f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.linear"), lighting->SetAttenuationLinear(0.09f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.quadratic"), lighting->SetAttenuationQuadratic(0.032f));
//
//	// Calculate the cos value based on an angle and this angle to the fragment shader
//	glUniform1f(glGetUniformLocation(shaderProgram, "light.cutoffAngle"), 
//		glm::cos(glm::radians(lighting->SetCutoffAngle(12.5f))));
//
//	glUniform1f(glGetUniformLocation(shaderProgram, "light.outerCutoffAngle"),
//		glm::cos(glm::radians(lighting->SetOuterCutoffAngle(15.5f))));
//
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].constant"), lighting->SetAttenuationConstant(1.0f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].constant"), lighting->SetAttenuationConstant(1.0f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].constant"), lighting->SetAttenuationConstant(1.0f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].constant"), lighting->SetAttenuationConstant(1.0f));
//
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].linear"), lighting->SetAttenuationLinear(0.09f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].linear"), lighting->SetAttenuationLinear(0.09f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].linear"), lighting->SetAttenuationLinear(0.09f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].linear"), lighting->SetAttenuationLinear(0.09f));
//
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
//	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
//
//	// To be able to draw in 3D, we will need a model matrix
//
//	/* The model matrix includes translations, scaling and /or rotations we want to make to transform all the object's
//	vertices to the global world space */
//
//	/* By multiplying the vertex coordinates with the model matrix, we are essentially converting the vertex
//	coordinates from local space to the world space. That means whatever we're doing using the model matrix, we
//	are forcing the vertex coordinates to represent the plane in world space rather than the space of itself. */
//	modelMatrix = glm::mat4(1.0f);
//
//	// Set the model matrix location to look for the modelMatrix uniform set inside the vertex shader GLSL file
//	modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
//
//	// Set the uniform model matrix location to equal to the modelMatrix value pointer
//	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
//
//	/* After the model matrix has been created and set up, we need to create a view matrix. We have to move
//	slightly backwards in the scene so that the objects are visible inside the OpenGL window (when we're in world
//	space, we are located at the origin (or in other words, the coordinate (0, 0, 0). */
//
//	/* The view matrix basically moves the entire scene around inversed (or reversed) to where we want the camera to
//	move. Because we want to move backwards and since OpenGL is a right-handed system we have to move in the positive
//	z-axis. This gives the impression that we're moving backwards. */
//	viewMatrix = glm::mat4(1.0f);
//	//viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.0f, -3.0f)); // Translate the scene in the reversed direction
//
//	/* The LookAt matrix defines a coordinate space using 3 perpendicular (or non-linear) axes and this is where
//	you can create a matrix with those 3 axes plus a translation vector, that way you can transform any vector to
//	to that coordinate space simply by multiplying it with the LookAt matrix. */
//
//	// LookAt function passes in 3 parameters, one for camera's position, two for target, and three for up vector
//	//viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//
//	// Call the initialize camera function before setting the view matrix or else the cubes won't render on the window
//	//camera->InitializeCamera();
//
//	/*viewMatrix = glm::lookAt(glm::vec3(camera->cameraX, 0.0, camera->cameraZ), glm::vec3(0.0, 0.0, 0.0),
//	glm::vec3(0.0, 1.0, 0.0)); */
//
//	// First we must set the camera position to the camera position vec3 we defined in the camera class
//
//	/* Second we must set the direction to add the camera's current position and the camera's direction vector.
//	Basically, this will make sure however we move, the camera will keep looking at the target direction. */
//	//viewMatrix = glm::lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);
//
//	viewMatrix = Camera::CameraLookAt(); // Use my very own LookAt function I defined in the Camera class
//
//	viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
//	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
//
//	/* The last thing we need to define is the projection matrix where we're going to use it in our scene */
//
//	/* The glm::perspective function passes in 4 float values: the first one passes in the field of view in the y
//	direction, the second one passes in the aspect ratio value, the third one passes in the near plane value (or the
//	distance from the viewer to the near clipping plane point (always positive, so most likely we need an unsigned int
//	reference here), and the fourth one passes in the far plane value (or the distance from the viewer to the far
//	clipping plane point (again, always positive so another potential unsigned int is mandatory here) */
//	projectionMatrix = glm::perspective(glm::radians(Camera::fieldOfView), aspect_ratio, near_plane, far_plane);
//
//	projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
//	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
//
//	glDrawArrays(GL_TRIANGLES, 0, 36);
//
//	// Loop through all the cubes positions array
//
//	for (unsigned int renderCubes = 0; renderCubes < cubePositions.size(); renderCubes++)
//	{
//		// To be able to draw in 3D, we will need a model matrix
//
//		/* The model matrix includes translations, scaling and /or rotations we want to make to transform all the object's
//		vertices to the global world space */
//
//		/* By multiplying the vertex coordinates with the model matrix, we are essentially converting the vertex
//		coordinates from local space to the world space. That means whatever we're doing using the model matrix, we
//		are forcing the vertex coordinates to represent the plane in world space rather than the space of itself. */
//		modelMatrix = glm::mat4(1.0f);
//
//		// Use the model matrix to place all the cube positions elements somewhere inside the window
//		modelMatrix = glm::translate(modelMatrix, cubePositions[renderCubes]);
//
//		// Create an angle variable to determine the angle that each cube should rotate at
//		float angle = 20.0f * renderCubes;
//
//		// Rotate each container by the angle they're equal to
//		modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
//
//		// Set the model matrix location to look for the modelMatrix uniform set inside the vertex shader GLSL file
//		modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
//
//		// Set the uniform model matrix location to equal to the modelMatrix value pointer
//		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
//
//		/* Make sure to draw arrays at the end of setting the model, view and projection matrices locations to render
//		the cube on the window */
//		glDrawArrays(GL_TRIANGLES, 0, 36);
//	}
//}
//
//void ShaderProgram::InitializeLightColor(float aspect_ratio, float near_plane, float far_plane)
//{
//	/* After the model matrix has been created and set up, we need to create a view matrix. We have to move
//	slightly backwards in the scene so that the objects are visible inside the OpenGL window (when we're in world
//	space, we are located at the origin (or in other words, the coordinate (0, 0, 0). */
//
//	/* The view matrix basically moves the entire scene around inversed (or reversed) to where we want the camera to
//	move. Because we want to move backwards and since OpenGL is a right-handed system we have to move in the positive
//	z-axis. This gives the impression that we're moving backwards. */
//	viewMatrix = glm::mat4(1.0f);
//	//viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.0f, -3.0f)); // Translate the scene in the reversed direction
//
//	/* The LookAt matrix defines a coordinate space using 3 perpendicular (or non-linear) axes and this is where
//	you can create a matrix with those 3 axes plus a translation vector, that way you can transform any vector to
//	to that coordinate space simply by multiplying it with the LookAt matrix. */
//
//	// LookAt function passes in 3 parameters, one for camera's position, two for target, and three for up vector
//	//viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//
//	// Call the initialize camera function before setting the view matrix or else the cubes won't render on the window
//	//camera->InitializeCamera();
//
//	/*viewMatrix = glm::lookAt(glm::vec3(camera->cameraX, 0.0, camera->cameraZ), glm::vec3(0.0, 0.0, 0.0),
//	glm::vec3(0.0, 1.0, 0.0)); */
//
//	// First we must set the camera position to the camera position vec3 we defined in the camera class
//
//	/* Second we must set the direction to add the camera's current position and the camera's direction vector.
//	Basically, this will make sure however we move, the camera will keep looking at the target direction. */
//	//viewMatrix = glm::lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);
//
//	viewMatrix = Camera::CameraLookAt(); // Use my very own LookAt function I defined in the Camera class
//
//	viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
//	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
//
//	/* The last thing we need to define is the projection matrix where we're going to use it in our scene */
//
//	/* The glm::perspective function passes in 4 float values: the first one passes in the field of view in the y
//	direction, the second one passes in the aspect ratio value, the third one passes in the near plane value (or the
//	distance from the viewer to the near clipping plane point (always positive, so most likely we need an unsigned int
//	reference here), and the fourth one passes in the far plane value (or the distance from the viewer to the far
//	clipping plane point (again, always positive so another potential unsigned int is mandatory here) */
//	projectionMatrix = glm::perspective(glm::radians(Camera::fieldOfView), aspect_ratio, near_plane, far_plane);
//
//	projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
//	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
//
//	// To be able to draw in 3D, we will need a model matrix
//
//	/* The model matrix includes translations, scaling and /or rotations we want to make to transform all the object's
//	vertices to the global world space */
//
//	/* By multiplying the vertex coordinates with the model matrix, we are essentially converting the vertex
//	coordinates from local space to the world space. That means whatever we're doing using the model matrix, we
//	are forcing the vertex coordinates to represent the plane in world space rather than the space of itself. */
//	for (unsigned int i = 0; i < pointLightPositions.size(); i++)
//	{
//		modelMatrix = glm::mat4(1.0f);
//		modelMatrix = glm::translate(modelMatrix, pointLightPositions[i]);
//		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
//
//		// Set the model matrix location to look for the modelMatrix uniform set inside the vertex shader GLSL file
//		modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
//
//		// Set the uniform model matrix location to equal to the modelMatrix value pointer
//		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
//
//		glDrawArrays(GL_TRIANGLES, 0, 36);
//	}
//}

// Use the directional, point and spot lights to determine the desert's lighting
/*void ShaderProgram::CreateDesertLighting()
{
	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.lightDirection"), 1,
		glm::value_ptr(lighting->SetDirectionalLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), lighting->SetShininessLighting(32.0f));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	// Set the point lighting ambient, diffuse and specular light uniforms
	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].ambientLight"), 2,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].diffuseLight"), 2,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].specularLight"), 2,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].ambientLight"), 3,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].diffuseLight"), 3,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].specularLight"), 3,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].ambientLight"), 4,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].diffuseLight"), 4,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].specularLight"), 4,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.5f, 1.5f, 1.5f))));

	// Set the spot lighting diffuse and specular light uniforms
	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.linear"), lighting->SetAttenuationLinear(0.09f));
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.quadratic"), lighting->SetAttenuationQuadratic(0.032f));

	// Calculate the cos value based on an angle and this angle to the fragment shader
	glUniform1f(glGetUniformLocation(shaderProgram, "light.cutoffAngle"),
		glm::cos(glm::radians(lighting->SetCutoffAngle(12.5f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "light.outerCutoffAngle"),
		glm::cos(glm::radians(lighting->SetOuterCutoffAngle(15.5f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].constant"), lighting->SetAttenuationConstant(1.0f));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].linear"), lighting->SetAttenuationLinear(0.09f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].linear"), lighting->SetAttenuationLinear(0.09f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].linear"), lighting->SetAttenuationLinear(0.09f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].linear"), lighting->SetAttenuationLinear(0.09f));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
}

void ShaderProgram::CreateFactoryLighting()
{
	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.lightDirection"), 1,
		glm::value_ptr(lighting->SetDirectionalLighting(glm::vec3(-0.2f, -1.0f, -0.3f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), lighting->SetShininessLighting(16.0f));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.23921568627f, 0.23921568627f, 0.80392156862f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	// Set the point lighting ambient, diffuse and specular light uniforms
	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.23921568627f, 0.23921568627f, 0.80392156862f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].ambientLight"), 2,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].diffuseLight"), 2,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.23921568627f, 0.23921568627f, 0.80392156862f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].specularLight"), 2,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].ambientLight"), 3,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].diffuseLight"), 3,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.23921568627f, 0.23921568627f, 0.80392156862f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].specularLight"), 3,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].ambientLight"), 4,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].diffuseLight"), 4,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.23921568627f, 0.23921568627f, 0.80392156862f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].specularLight"), 4,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	// Set the spot lighting diffuse and specular light uniforms
	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.23921568627f, 0.23921568627f, 0.80392156862f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.23921568627f, 0.23921568627f, 0.80392156862f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(2.0f, 2.0f, 2.0f))));

	// Try to cover the light distance of up to 50
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.linear"), lighting->SetAttenuationLinear(0.09f));
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.quadratic"), lighting->SetAttenuationQuadratic(0.032f));

	// Calculate the cos value based on an angle and this angle to the fragment shader
	glUniform1f(glGetUniformLocation(shaderProgram, "light.cutoffAngle"), 
		glm::cos(glm::radians(lighting->SetCutoffAngle(12.5f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "light.outerCutoffAngle"),
		glm::cos(glm::radians(lighting->SetOuterCutoffAngle(15.5f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].constant"), lighting->SetAttenuationConstant(1.0f));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].linear"), lighting->SetAttenuationLinear(0.09f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].linear"), lighting->SetAttenuationLinear(0.09f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].linear"), lighting->SetAttenuationLinear(0.09f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].linear"), lighting->SetAttenuationLinear(0.09f));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].quadratic"), lighting->SetAttenuationQuadratic(0.032f));
}

void ShaderProgram::CreateHorrorLighting()
{
	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.lightDirection"), 1,
		glm::value_ptr(lighting->SetDirectionalLighting(glm::vec3(-0.2f, -1.0f, -0.3f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), lighting->SetShininessLighting(32.0f));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.41568627451f, 0.11372549019f, 0.11372549019f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	// Set the point lighting ambient, diffuse and specular light uniforms
	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.41568627451f, 0.11372549019f, 0.11372549019f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].ambientLight"), 2,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].diffuseLight"), 2,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.41568627451f, 0.11372549019f, 0.11372549019f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].specularLight"), 2,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].ambientLight"), 3,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].diffuseLight"), 3,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.41568627451f, 0.11372549019f, 0.11372549019f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].specularLight"), 3,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].ambientLight"), 4,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].diffuseLight"), 4,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.41568627451f, 0.11372549019f, 0.11372549019f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].specularLight"), 4,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	// Set the spot lighting diffuse and specular light uniforms
	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.41568627451f, 0.11372549019f, 0.11372549019f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	// Try to cover the light distance of up to 50
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.linear"), lighting->SetAttenuationLinear(0.22f));
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.quadratic"), lighting->SetAttenuationQuadratic(0.2f));

	// Calculate the cos value based on an angle and this angle to the fragment shader
	glUniform1f(glGetUniformLocation(shaderProgram, "light.cutoffAngle"),
		glm::cos(glm::radians(lighting->SetCutoffAngle(12.5f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "light.outerCutoffAngle"),
		glm::cos(glm::radians(lighting->SetOuterCutoffAngle(15.5f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].constant"), lighting->SetAttenuationConstant(1.0f));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].linear"), lighting->SetAttenuationLinear(0.22f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].linear"), lighting->SetAttenuationLinear(0.22f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].linear"), lighting->SetAttenuationLinear(0.22f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].linear"), lighting->SetAttenuationLinear(0.22f));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].quadratic"), lighting->SetAttenuationQuadratic(0.2f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].quadratic"), lighting->SetAttenuationQuadratic(0.2f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].quadratic"), lighting->SetAttenuationQuadratic(0.2f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].quadratic"), lighting->SetAttenuationQuadratic(0.2f));
}

void ShaderProgram::CreateBioChemicalLabLighting()
{
	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.lightDirection"), 1,
		glm::value_ptr(lighting->SetDirectionalLighting(glm::vec3(-0.2f, -1.0f, -0.3f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), lighting->SetShininessLighting(32.0f));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.55686274509f, 0.88235294117f, 0.10980392156f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "directionalLight.specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	// Set the point lighting ambient, diffuse and specular light uniforms
	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.55686274509f, 0.88235294117f, 0.10980392156f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[0].specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].ambientLight"), 2,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].diffuseLight"), 2,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.55686274509f, 0.88235294117f, 0.10980392156f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[1].specularLight"), 2,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].ambientLight"), 3,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].diffuseLight"), 3,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.55686274509f, 0.88235294117f, 0.10980392156f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[2].specularLight"), 3,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].ambientLight"), 4,
		glm::value_ptr(lighting->SetIntensityAmbientLighting(glm::vec3(0.05f, 0.05f, 0.05f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].diffuseLight"), 4,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.55686274509f, 0.88235294117f, 0.10980392156f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "pointLight[3].specularLight"), 4,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	// Set the spot lighting diffuse and specular light uniforms
	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.ambientLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.0f, 0.0f, 0.0f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.diffuseLight"), 1,
		glm::value_ptr(lighting->SetIntensityDiffuseLighting(glm::vec3(0.55686274509f, 0.88235294117f, 0.10980392156f))));

	glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.specularLight"), 1,
		glm::value_ptr(lighting->SetIntensitySpecularLighting(glm::vec3(1.0f, 1.0f, 1.0f))));

	// Try to cover the light distance of up to 50
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.linear"), lighting->SetAttenuationLinear(0.045f));
	glUniform1f(glGetUniformLocation(shaderProgram, "attenuation.quadratic"), lighting->SetAttenuationQuadratic(0.0075f));

	// Calculate the cos value based on an angle and this angle to the fragment shader
	glUniform1f(glGetUniformLocation(shaderProgram, "light.cutoffAngle"),
		glm::cos(glm::radians(lighting->SetCutoffAngle(12.5f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "light.outerCutoffAngle"),
		glm::cos(glm::radians(lighting->SetOuterCutoffAngle(15.5f))));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].constant"), lighting->SetAttenuationConstant(1.0f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].constant"), lighting->SetAttenuationConstant(1.0f));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].linear"), lighting->SetAttenuationLinear(0.045f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].linear"), lighting->SetAttenuationLinear(0.045f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].linear"), lighting->SetAttenuationLinear(0.045f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].linear"), lighting->SetAttenuationLinear(0.045f));

	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].quadratic"), lighting->SetAttenuationQuadratic(0.0075f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].quadratic"), lighting->SetAttenuationQuadratic(0.0075f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[2].quadratic"), lighting->SetAttenuationQuadratic(0.0075f));
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[3].quadratic"), lighting->SetAttenuationQuadratic(0.0075f));
} */