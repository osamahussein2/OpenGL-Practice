#include "SSAO.h"
#include "Camera.h"
#include <random>

SSAO* SSAO::ssaoInstance = NULL;

/* Ambient lighting is a fixed light constant we add to the overall lighting of a scene to simulate the scattering of light.
One type of indirect lighting approximation is called ambient occlusion that tries to approximate indirect lighting by 
darkening creases, holes, and surfaces that are close to each other. */

/* Screen-space ambient occlusion (SSAO) technique uses a scene’s depth buffer in screen-space to determine the amount of
occlusion instead of real geometrical data. This approach is incredibly fast compared to real ambient occlusion and gives 
plausible results, making it the de-facto standard for approximating real-time ambient occlusion */

/* For each fragment on a screen-filled quad we calculate an occlusion factor based on the fragment’s surrounding depth values. 
The occlusion factor is then used to reduce or nullify the fragment’s ambient lighting component. The occlusion factor is 
obtained by taking multiple depth samples in a sphere sample kernel surrounding the fragment position and compare each of the 
samples with the current fragment’s depth value. The number of samples that have a higher depth value than the fragment’s 
depth represents the occlusion factor */

/* SSAO requires geometrical info as we need some way to determine the occlusion factor of a fragment.

	- A per-fragment position vector
	- A per-fragment normal vector
	- A per-fragment albedo color
	- A sample kernel
	- A per-fragment random rotation vector used to rotate the sample kernel 
*/

SSAO::SSAO() : ssaoShaders{ new ShaderProgram(), new ShaderProgram(), new ShaderProgram(), new ShaderProgram() }, 
cubeVAO(0), cubeVBO(0), quadVAO(0), quadVBO(0)
{
}

SSAO::~SSAO()
{
	array<unsigned int, 2> VAOs = { quadVAO, cubeVAO };
	array<unsigned int, 2> VBOs = { quadVBO, cubeVBO };
	array<unsigned int, 8> frameBuffers = { gBuffer, gPosition, gNormal, gAlbedo, ssaoFBO, ssaoBlurFBO,
		ssaoColorBuffer, ssaoColorBufferBlur };

	for (unsigned int i = 0; i < VAOs.size(); i++)
	{
		glDeleteVertexArrays(1, &VAOs[i]);
	}

	for (unsigned int i = 0; i < VBOs.size(); i++)
	{
		glDeleteBuffers(1, &VBOs[i]);
	}

	for (unsigned int i = 0; i < frameBuffers.size(); i++)
	{
		glDeleteFramebuffers(1, &frameBuffers[i]);
	}

	for (unsigned int i = 0; i < attachments.size(); i++)
	{
		glDeleteFramebuffers(1, &attachments[i]);
	}

	glDeleteRenderbuffers(1, &rboDepth);

	ssaoInstance = NULL;

	for (int i = 0; i < ssaoShaders.size(); i++)
	{
		delete ssaoShaders[i];
	}
}

SSAO* SSAO::Instance()
{
	if (ssaoInstance == NULL)
	{
		ssaoInstance = new SSAO();
		return ssaoInstance;
	}

	return ssaoInstance;
}

void SSAO::InitializeSSAO()
{
	glEnable(GL_DEPTH_TEST);

	// Geometry pass for SSAO shaders
	ssaoShaders[0]->InitializeShaderProgram(new VertexShaderLoader("ssaoGeometryVertexShader.glsl"), 
		new FragmentShaderLoader("ssaoGeometryFragmentShader.glsl"));

	// Lighting pass for SSAO shaders
	ssaoShaders[1]->InitializeShaderProgram(new VertexShaderLoader("ssaoVertexShader.glsl"),
		new FragmentShaderLoader("ssaoLightingFragmentShader.glsl"));

	// SSAO shaders
	ssaoShaders[2]->InitializeShaderProgram(new VertexShaderLoader("ssaoVertexShader.glsl"),
		new FragmentShaderLoader("ssaoFragmentShader.glsl"));

	// Blur for SSAO shaders
	ssaoShaders[3]->InitializeShaderProgram(new VertexShaderLoader("ssaoVertexShader.glsl"),
		new FragmentShaderLoader("ssaoBlurFragmentShader.glsl"));

	backpack = new Model("Models/Backpack/backpack.obj");

	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 960, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 960, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// color + specular color buffer
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 960, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	// Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	for (int i = 0; i < attachments.size(); i++)
	{
		glDrawBuffers(3, &attachments[i]);
	}

	// Create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 960);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) cout << "Framebuffer not complete!" << endl;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create framebuffer to hold SSAO processing stage
	glGenFramebuffers(1, &ssaoFBO);  
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	// SSAO color buffer
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1280, 960, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
		cout << "SSAO Framebuffer not complete!" << endl;

	// Blur stage
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1280, 960, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
		cout << "SSAO Blur Framebuffer not complete!" << endl;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// generate sample kernel (generating random floats between 0.0 and 1.0)
	uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	default_random_engine generator;

	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = normalize(sample);
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0f;

		// Scale samples so that they're more aligned to center of kernel
		scale = 0.1f + scale * scale * (1.0f - 0.1f);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// Generate noise texture
	for (unsigned int i = 0; i < 16; i++)
	{
		// rotate around z-axis (in tangent space)
		vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
		ssaoNoise.push_back(noise);
	}
	
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	lightPos = vec3(2.0, 4.0, -2.0);
	lightColor = vec3(0.2, 0.2, 0.7);

	glUseProgram(ssaoShaders[1]->shaderProgram);
	glUniform1i(glGetUniformLocation(ssaoShaders[1]->shaderProgram, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(ssaoShaders[1]->shaderProgram, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(ssaoShaders[1]->shaderProgram, "gAlbedo"), 2);
	glUniform1i(glGetUniformLocation(ssaoShaders[1]->shaderProgram, "ssao"), 3);

	glUseProgram(ssaoShaders[2]->shaderProgram);
	glUniform1i(glGetUniformLocation(ssaoShaders[2]->shaderProgram, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(ssaoShaders[2]->shaderProgram, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(ssaoShaders[2]->shaderProgram, "texNoise"), 2);
	
	glUseProgram(ssaoShaders[3]->shaderProgram);
	glUniform1i(glGetUniformLocation(ssaoShaders[3]->shaderProgram, "ssaoInput"), 0);
}

void SSAO::RenderSSAO()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Geometry pass: render scene's geometry/color data into gbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 projection = perspective(glm::radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 50.0f);
	mat4 view = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);
	mat4 model = mat4(1.0f);

	glUseProgram(ssaoShaders[0]->shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(ssaoShaders[0]->shaderProgram, "projection"), 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(ssaoShaders[0]->shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

	// Room cube
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0, 7.0f, 0.0f));
	model = glm::scale(model, glm::vec3(7.5f, 7.5f, 7.5f));

	glUniformMatrix4fv(glGetUniformLocation(ssaoShaders[0]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));

	// Invert normals as we're inside the cube
	glUniform1i(glGetUniformLocation(ssaoShaders[0]->shaderProgram, "invertedNormals"), 1);
	RenderCube();
	glUniform1i(glGetUniformLocation(ssaoShaders[0]->shaderProgram, "invertedNormals"), 0);
	
	// Backpack model on the floor
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
	model = glm::scale(model, glm::vec3(1.0f));

	glUniformMatrix4fv(glGetUniformLocation(ssaoShaders[0]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
	backpack->DrawModel(ssaoShaders[0]);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Generate SSAO texture
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(ssaoShaders[2]->shaderProgram);

	// Send kernel + rotation 
	for (unsigned int i = 0; i < 64; ++i)
	{
		glUniform3fv(glGetUniformLocation(ssaoShaders[2]->shaderProgram,
			("samples[" + std::to_string(i) + "]").c_str()), 1, value_ptr(ssaoKernel[i]));
	}

	glUniformMatrix4fv(glGetUniformLocation(ssaoShaders[2]->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(projection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);

	RenderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Blur SSAO texture to remove noise
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(ssaoShaders[3]->shaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

	RenderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Send light relevant uniforms
	glUseProgram(ssaoShaders[1]->shaderProgram);
	vec3 lightPosView = vec3(view * vec4(lightPos, 1.0));

	glUniform3fv(glGetUniformLocation(ssaoShaders[1]->shaderProgram, "light.Position"), 1, value_ptr(lightPosView));
	glUniform3fv(glGetUniformLocation(ssaoShaders[1]->shaderProgram, "light.Color"), 1, value_ptr(lightColor));

	// Update attenuation parameters
	const float linear = 0.09f;
	const float quadratic = 0.032f;

	glUniform1f(glGetUniformLocation(ssaoShaders[1]->shaderProgram, "light.Linear"), linear);
	glUniform1f(glGetUniformLocation(ssaoShaders[1]->shaderProgram, "light.Quadratic"), quadratic);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);

	glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

	RenderQuad();
}

void SSAO::RenderQuad()
{
	if (quadVAO == 0)
	{
		array<float, 20> quadVertices = 
		{
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);

		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void SSAO::RenderCube()
{
	if (cubeVAO == 0)
	{
		array<float, 288> vertices = {
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

		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}