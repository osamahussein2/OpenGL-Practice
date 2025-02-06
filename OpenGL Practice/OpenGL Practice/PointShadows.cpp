#include "PointShadows.h"
#include "Camera.h"

PointShadows* PointShadows::pointShadowsInstance = 0;

PointShadows::PointShadows()
{
	pointShadowShaderProgram = 
	{ 
		new ShaderProgram(), 
		new ShaderProgram() 
	};

	vertexShaderLoader = 
	{
		new VertexShaderLoader("PointShadowsVertexShader.glsl"),
		new VertexShaderLoader("PointShadowsDepthVertexShader.glsl")
	};

	fragmentShaderLoader =
	{
		new FragmentShaderLoader("PointShadowsFragmentShader.glsl"),
		new FragmentShaderLoader("PointShadowsDepthFragmentShader.glsl")
	};

	geometryShaderLoader = new GeometryShader("PointShadowsDepthGeometryShader.glsl");

	cubeVAO = NULL;
	cubeVBO = NULL;

	shadows = false;
}

PointShadows::~PointShadows()
{
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
}

PointShadows* PointShadows::Instance()
{
	if (pointShadowsInstance == 0)
	{
		pointShadowsInstance = new PointShadows();
		return pointShadowsInstance;
	}

	return pointShadowsInstance;
}

void PointShadows::InitializePointShadows()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Load the point shadow shaders
	pointShadowShaderProgram[0]->InitializeShaderProgram(vertexShaderLoader[0], fragmentShaderLoader[0]);

	// Load the point shadow depth shaders along with the depth geometry shader
	pointShadowShaderProgram[1]->InitializeShaderProgram(vertexShaderLoader[1], fragmentShaderLoader[1], 
		geometryShaderLoader);
}

void PointShadows::InitializeTexture(const char* path_)
{
	glGenTextures(1, &woodTexture);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path_, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path_ << std::endl;
		stbi_image_free(data);
	}
}

void PointShadows::InitializeDepthCubemapTexture()
{
	glGenFramebuffers(1, &depthMapFBO);

	// Create depth cubemap texture
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

	// Assign each of the single cubemap faces a 2D depth-valued texture image
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void PointShadows::InitializeFramebuffers()
{
	// Attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	/* Since I have a usable geometry shader, that allows us to render to all faces in a single pass, we can directly 
	attach the cubemap as a framebuffer’s depth attachment with glFramebufferTexture */
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);

	/* We only care about depth values when generating a depth cubemap so we have to explicitly tell OpenGL this 
	framebuffer object does not render to a color buffer */
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PointShadows::InitializeTextureUniformShaders()
{
	glUseProgram(pointShadowShaderProgram[0]->shaderProgram);

	glUniform1i(glGetUniformLocation(pointShadowShaderProgram[0]->shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(pointShadowShaderProgram[0]->shaderProgram, "depthMap"), 1);

	lightPosition = vec3(0.0f, 0.0f, 0.0f);
}

void PointShadows::ShowPointShadows()
{
	// Move light position over time
	lightPosition.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create depth cubemap transformation matrices
	float near_plane = 1.0f;
	float far_plane = 25.0f;

	/* By setting this to 90 degrees we make sure the viewing field is exactly large enough to fill a single face of the 
	cubemap such that all faces align correctly to each other at the edges */
	mat4 shadowProjection = perspective(radians(90.0f), float(SHADOW_WIDTH / SHADOW_HEIGHT), near_plane, far_plane);

	/* As the projection matrix does not change per direction we can re-use it for each of the 6 transformation matrices. 
	We do need a different view matrix per direction. With glm::lookAt we create 6 view directions, each looking at one 
	face direction of the cubemap in the order: right, left, top, bottom, near and far */

	/* Create 6 view matrices and multiply them with the projection matrix to get a total of 6 different light space 
	transformation matrices. The target parameter of glm::lookAt each looks into the direction of a single cubemap face */
	vector<mat4> shadowTransforms;

	shadowTransforms.push_back(shadowProjection * lookAt(lightPosition, lightPosition + vec3(1.0f, 0.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProjection * lookAt(lightPosition, lightPosition + vec3(-1.0f, 0.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProjection * lookAt(lightPosition, lightPosition + vec3(0.0f, 1.0f, 0.0f),
		vec3(0.0f, 0.0f, 1.0f)));
	shadowTransforms.push_back(shadowProjection * lookAt(lightPosition, lightPosition + vec3(0.0f, -1.0f, 0.0f),
		vec3(0.0f, 0.0f, -1.0f)));
	shadowTransforms.push_back(shadowProjection * lookAt(lightPosition, lightPosition + vec3(0.0f, 0.0f, 1.0f),
		vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProjection * lookAt(lightPosition, lightPosition + vec3(0.0f, 0.0f, -1.0f),
		vec3(0.0f, -1.0f, 0.0f)));

	// Render scene to depth cubemap
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(pointShadowShaderProgram[1]->shaderProgram);

	for (unsigned int i = 0; i < 6; i++)
	{
		const string& name = "shadowMatrices[" + to_string(i) + "]";

		glUniformMatrix4fv(glGetUniformLocation(pointShadowShaderProgram[1]->shaderProgram, name.c_str()), 
			1, GL_FALSE, value_ptr(shadowTransforms[i]));
	}

	glUniform1f(glGetUniformLocation(pointShadowShaderProgram[1]->shaderProgram, "farPlane"), far_plane);
	glUniform3fv(glGetUniformLocation(pointShadowShaderProgram[1]->shaderProgram, "lightPosition"), 1, 
		value_ptr(lightPosition));

	RenderScene(pointShadowShaderProgram[1]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Render scene as normal
	glViewport(0, 0, 1280, 960);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pointShadowShaderProgram[0]->shaderProgram);

	glm::mat4 projection = glm::perspective(glm::radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 100.0f);

	glm::mat4 view = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);

	glUniformMatrix4fv(glGetUniformLocation(pointShadowShaderProgram[0]->shaderProgram, "projectionMatrix"), 1, 
		GL_FALSE, value_ptr(projection));

	glUniformMatrix4fv(glGetUniformLocation(pointShadowShaderProgram[0]->shaderProgram, "viewMatrix"), 1,
		GL_FALSE, value_ptr(view));

	// Set lighting uniforms
	glUniform3fv(glGetUniformLocation(pointShadowShaderProgram[0]->shaderProgram, "lightPosition"), 1, 
		value_ptr(lightPosition));

	glUniform3fv(glGetUniformLocation(pointShadowShaderProgram[0]->shaderProgram, "viewPosition"), 1, 
		value_ptr(Camera::cameraPosition));

	// enable/disable shadows by pressing 'SPACE'
	glUniform1i(glGetUniformLocation(pointShadowShaderProgram[0]->shaderProgram, "shadows"), shadows);

	glUniform1f(glGetUniformLocation(pointShadowShaderProgram[0]->shaderProgram, "farPlane"), far_plane);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

	RenderScene(pointShadowShaderProgram[0]);
}

void PointShadows::RenderScene(ShaderProgram* shaderProgram_)
{
	// Room cube
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(5.0f));

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_->shaderProgram, "modelMatrix"), 1,
		GL_FALSE, value_ptr(model));

	/* Disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal 
	culling methods */
	glDisable(GL_CULL_FACE);

	// A small little hack to invert normals when drawing cube from the inside so lighting still works
	glUniform1i(glGetUniformLocation(shaderProgram_->shaderProgram, "reverseNormals"), 1);

	RenderCube();

	// Disable reverse normals
	glUniform1i(glGetUniformLocation(shaderProgram_->shaderProgram, "reverseNormals"), 0);
	glEnable(GL_CULL_FACE);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_->shaderProgram, "modelMatrix"), 1,
		GL_FALSE, value_ptr(model));

	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.75f));

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_->shaderProgram, "modelMatrix"), 1,
		GL_FALSE, value_ptr(model));

	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_->shaderProgram, "modelMatrix"), 1,
		GL_FALSE, value_ptr(model));

	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
	model = glm::scale(model, glm::vec3(0.5f));

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_->shaderProgram, "modelMatrix"), 1,
		GL_FALSE, value_ptr(model));

	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.75f));

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_->shaderProgram, "modelMatrix"), 1,
		GL_FALSE, value_ptr(model));

	RenderCube();
}

void PointShadows::RenderCube()
{
	float vertices[] = {
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left

		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left

		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right

		// right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left 

		 // bottom face
		 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right

		 // top face
		 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	// Fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Link vertex attributes
	glBindVertexArray(cubeVAO);

	glEnableVertexAttribArray(POSITION);
	glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(NORMAL);
	glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(TEXTURE);
	glVertexAttribPointer(TEXTURE, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Render cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}