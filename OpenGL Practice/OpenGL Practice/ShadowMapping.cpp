#include "ShadowMapping.h"
#include "Camera.h"

/* Shadows are a result of the absence of light due to occlusion. When a light source’s light rays do not hit an object 
because it gets occluded (blocked) by some other object, the object is in shadow */

/* One technique used by most videogames that gives decent results and is relatively easy to
implement is shadow mapping. The idea behind shadow mapping is to render the scene from the light’s point of view and 
everything we see from the light’s perspective is lit and everything we can’t see must be in shadow.  */

/* It's possible to sample the closest depth values as seen from the light’s perspective. After all, the depth values show 
the first fragment visible from the light’s perspective. We store all these depth values in a texture that we call a depth 
map (z distance stored in texture) or shadow map */

/* Using the depth values stored in the depth map we find the closest point and use that to determine whether fragments are 
in shadow. We create the depth map by rendering the scene (from the light’s perspective) using a view and projection matrix 
specific to that light source. This projection and view matrix together form a transformation T that transforms any 3D
position to the light’s (visible) coordinate space */

ShadowMapping* ShadowMapping::instance = 0;

ShadowMapping::ShadowMapping()
{
	shadowMappingShaderProgram[0] = new ShaderProgram();
	shadowMappingShaderProgram[1] = new ShaderProgram();
	shadowMappingShaderProgram[2] = new ShaderProgram();
}

ShadowMapping::~ShadowMapping()
{
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
}

ShadowMapping* ShadowMapping::Instance()
{
	if (instance == 0)
	{
		instance = new ShadowMapping();
		return instance;
	}

	return instance;
}

void ShadowMapping::InitializePlaneVertices()
{
	glEnable(GL_DEPTH_TEST);

	shadowMappingShaderProgram[0]->InitializeShaderProgram(new VertexShaderLoader("SimpleDepthVertexShader.glsl"),
		new FragmentShaderLoader("SimpleDepthFragmentShader.glsl"));

	shadowMappingShaderProgram[1]->InitializeShaderProgram(new VertexShaderLoader("QuadDepthVertexShader.glsl"),
		new FragmentShaderLoader("QuadDepthFragmentShader.glsl"));

	// Shadow Mapping Part 2
	shadowMappingShaderProgram[2]->InitializeShaderProgram(new VertexShaderLoader("ShadowMappingVertexShader.glsl"),
		new FragmentShaderLoader("ShadowMappingFragmentShader.glsl"));

	planeVertices = 
	{
		// positions            // normals         // texcoords
		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);

	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(vertexAttributes.positionAttribute);
	glVertexAttribPointer(vertexAttributes.positionAttribute, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(vertexAttributes.normalAttribute);
	glVertexAttribPointer(vertexAttributes.normalAttribute, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
		(void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(vertexAttributes.textureAttribute);
	glVertexAttribPointer(vertexAttributes.textureAttribute, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
		(void*)(6 * sizeof(float)));

	glBindVertexArray(0);
}

void ShadowMapping::InitializeTexture(char const* path)
{
	glGenTextures(1, &woodTexture);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
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

		// for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes 
		// texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
}

void ShadowMapping::InitializeFramebuffers()
{
	/* The depth map is the depth texture as rendered from the light’s perspective that will be used for testing for shadows.
	Because we need to store the rendered result of a scene into a texture we’re going to need framebuffers again */
	glGenFramebuffers(1, &depthMapFBO);

	// Create 2D texture that will be used as framebuffer's depth buffer
	// Because we only care about depth values we specify the texture’s formats as GL_DEPTH_COMPONENT
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Shadow Mapping Part 3
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

	/* A framebuffer object however is not complete without a color buffer so we need to explicitly tell OpenGL we’re not
	going to render any color data. We do this by setting both the read and draw buffer to GL_NONE with glDrawBuffer and
	glReadbuffer */
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Use the shadow mapping shaders (Shadow Mapping Part 2)
	glUseProgram(shadowMappingShaderProgram[2]->shaderProgram);
	glUniform1i(glGetUniformLocation(shadowMappingShaderProgram[2]->shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shadowMappingShaderProgram[2]->shaderProgram, "shadowMap"), 1);

	// Use the quad depth shaders
	glUseProgram(shadowMappingShaderProgram[1]->shaderProgram);
	glUniform1i(glGetUniformLocation(shadowMappingShaderProgram[1]->shaderProgram, "depthMap"), 0);

	// lighting info
	lightPosition = vec3(-2.0f, 4.0f, -1.0f);
}

void ShadowMapping::UseShaderProgram()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 1. first render to depth map

	/* Because we’re modelling a directional light source, all its light rays are parallel.For this reason, we’re going to
	use an orthographic projection matrix for the light source where there is no perspective deform */
	float near_plane = 1.0f, far_plane = 7.5f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	/* When using a perspective projection matrix, changing the light position as the current light position isn't enough to 
	reflect the whole scene */
	//lightProjection = glm::perspective(glm::radians(45.0f), GLfloat(SHADOW_WIDTH / SHADOW_HEIGHT), near_plane, far_plane);

	/* Because a projection matrix indirectly determines the range of what is visible (e.g. what is not clipped) you want
	to make sure the size of the projection frustum correctly contains the objects you want to be in the depth map. When
	objects or fragments are not in the depth map they will not produce shadows */

	/* To create a view matrix to transform each object so they’re visible from the light’s point of view, we’re going to
	use the infamous glm::lookAt function; this time with the light source’s position looking at the scene’s center */
	/*glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)); Shadow Mapping Part 1 */

	// Shadow Mapping Part 2
	glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

	/* Combining these two gives us a light space transformation matrix that transforms each worldspace vector into the
	space as visible from the light source; exactly what we need to render the depth map. */
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	glUseProgram(shadowMappingShaderProgram[0]->shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(shadowMappingShaderProgram[0]->shaderProgram, "lightSpaceMatrix"), 1,
		GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTexture);
	RenderScene(shadowMappingShaderProgram[0]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Reset viewport

	/* Because shadow maps often have a different resolution compared to what we originally render the scene in (usually the
	window resolution), we need to change the viewport parameters to accommodate for the size of the shadow map. If we
	forget to update the viewport parameters, the resulting depth map will be either incomplete or too small */
	glViewport(0, 0, 1280, 960);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2. Render scene as normal with the generated depth/shadow mapping

	// Shadow Mapping Part 2
	glUseProgram(shadowMappingShaderProgram[2]->shaderProgram);

	glm::mat4 projection = glm::perspective(glm::radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);

	glUniformMatrix4fv(glGetUniformLocation(shadowMappingShaderProgram[2]->shaderProgram, "projection"), 1, 
		GL_FALSE, value_ptr(projection));

	glUniformMatrix4fv(glGetUniformLocation(shadowMappingShaderProgram[2]->shaderProgram, "view"), 1, GL_FALSE, 
		value_ptr(view));

	// set light uniforms
	glUniform3fv(glGetUniformLocation(shadowMappingShaderProgram[2]->shaderProgram, "viewPosition"), 1, 
		value_ptr(Camera::cameraPosition));

	glUniform3fv(glGetUniformLocation(shadowMappingShaderProgram[2]->shaderProgram, "lightPosition"), 1, 
		value_ptr(lightPosition));

	glUniformMatrix4fv(glGetUniformLocation(shadowMappingShaderProgram[2]->shaderProgram, "lightSpaceMatrix"), 1, 
		GL_FALSE, value_ptr(lightSpaceMatrix));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	// Peter panning is when objects seem slightly detached from their shadows
	// To fix peter panning we cull all front faces during the shadow map generation by enabling GL_CULL_FACE first
	glCullFace(GL_FRONT);
	RenderScene(shadowMappingShaderProgram[2]);
	glCullFace(GL_BACK); // Reset original culling face

	// Render the depth map to quad for visual debugging
	glUseProgram(shadowMappingShaderProgram[1]->shaderProgram);
	glUniform1f(glGetUniformLocation(shadowMappingShaderProgram[1]->shaderProgram, "nearPlane"), near_plane);
	glUniform1f(glGetUniformLocation(shadowMappingShaderProgram[1]->shaderProgram, "farPlane"), far_plane);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	//RenderQuad(); // Commented out for Shadow Mapping Part 2
}

void ShadowMapping::RenderScene(ShaderProgram* shaderProgram_)
{
	// floor
	glm::mat4 model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_->shaderProgram, "modelMatrix"), 1, GL_FALSE, value_ptr(model));
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_->shaderProgram, "modelMatrix"), 1, GL_FALSE, value_ptr(model));
	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_->shaderProgram, "modelMatrix"), 1, GL_FALSE, value_ptr(model));
	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_->shaderProgram, "modelMatrix"), 1, GL_FALSE, value_ptr(model));
	RenderCube();
}

void ShadowMapping::RenderCube()
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

	unsigned int cubeVAO, cubeVBO;

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);
	glEnableVertexAttribArray(vertexAttributes.positionAttribute);
	glVertexAttribPointer(vertexAttributes.positionAttribute, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(vertexAttributes.normalAttribute);
	glVertexAttribPointer(vertexAttributes.normalAttribute, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
		(void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(vertexAttributes.textureAttribute);
	glVertexAttribPointer(vertexAttributes.textureAttribute, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
		(void*)(6 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void ShadowMapping::RenderQuad()
{
	float quadVertices[] = {
		// positions        // texture coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	unsigned int quadVAO, quadVBO;

	// setup plane VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(vertexAttributes.positionAttribute);
	glVertexAttribPointer(vertexAttributes.positionAttribute, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
