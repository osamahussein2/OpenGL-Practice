#include "DeferredShading.h"
#include "Camera.h"

DeferredShading* DeferredShading::deferredShadingInstance = NULL;

/* Deferred shading is based on the idea that we defer or postpone most of the heavy rendering (like lighting) to a later 
stage. Deferred shading consists of two passes: in the first pass, called the geometry pass, we render the scene once and 
retrieve all kinds of geometrical information from the objects that we store in a collection of textures called the G-buffer. 
The geometric information of a scene stored in the G-buffer is then later used for (more complex) lighting calculations */

/* We use the textures from the G-buffer in a second pass called the lighting pass where we render a screen-filled quad and 
calculate the scene’s lighting for each fragment using the geometrical information stored in the G-buffer; pixel by pixel we 
iterate over the G-buffer */

// The G-buffer is the collective term of all textures used to store lighting-relevant data for the final lighting pass

/* The data we’ll need to store of each fragment is a position vector, a normal vector, a color vector, and a specular 
intensity value. In the geometry pass we need to render all objects of the scene and store these data components in the 
G-buffer */

/* By itself (without light volumes), deferred shading is a nice optimization as each pixel only runs a single fragment 
shader, compared to forward rendering where we’d often run the fragment shader multiple times per pixel. Deferred rendering 
does come with a few disadvantages though: a large memory overhead, no MSAA, and blending still has to be done with forward 
rendering */

DeferredShading::DeferredShading() : deferredShadings{ new ShaderProgram(), new ShaderProgram(), new ShaderProgram() },
quadVAO(0), quadVBO(0), cubeVAO(0), cubeVBO(0), backpack(nullptr)
{
}

DeferredShading::~DeferredShading()
{
	array<unsigned int, 2> VAOs = { quadVAO, cubeVAO };
	array<unsigned int, 2> VBOs = { quadVBO, cubeVBO };
	array<unsigned int, 4> frameBuffers = { gBuffer, gPosition, gNormal, gAlbedoSpec };

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

	objectPositions.clear();
	lightPositions.clear();
	lightColors.clear();

	deferredShadingInstance = NULL;

	deferredShadings = { nullptr, nullptr, nullptr };
}

DeferredShading* DeferredShading::Instance()
{
	if (deferredShadingInstance == NULL)
	{
		deferredShadingInstance = new DeferredShading();
		return deferredShadingInstance;
	}

	return deferredShadingInstance;
}

void DeferredShading::InitializeDeferredShading()
{
	glEnable(GL_DEPTH_TEST);

	// Geometry pass for deferred shading
	deferredShadings[0]->InitializeShaderProgram(new VertexShaderLoader("gBufferVertexShader.glsl"),
		new FragmentShaderLoader("gBufferFragmentShader.glsl"));

	// Lighting pass for deferred shading
	deferredShadings[1]->InitializeShaderProgram(new VertexShaderLoader("DeferredShadingVertexShader.glsl"),
		new FragmentShaderLoader("DeferredShadingFragmentShader.glsl"));

	// Light box for deferred shading
	deferredShadings[2]->InitializeShaderProgram(new VertexShaderLoader("DeferredLightboxVertexShader.glsl"),
		new FragmentShaderLoader("DeferredLightboxFragmentShader.glsl"));

	backpack = new Model("Models/Backpack/backpack.obj");

	objectPositions.push_back(vec3(-3.0, -0.5, -3.0));
	objectPositions.push_back(vec3(0.0, -0.5, -3.0));
	objectPositions.push_back(vec3(3.0, -0.5, -3.0));
	objectPositions.push_back(vec3(-3.0, -0.5, 0.0));
	objectPositions.push_back(vec3(0.0, -0.5, 0.0));
	objectPositions.push_back(vec3(3.0, -0.5, 0.0));
	objectPositions.push_back(vec3(-3.0, -0.5, 3.0));
	objectPositions.push_back(vec3(0.0, -0.5, 3.0));
	objectPositions.push_back(vec3(3.0, -0.5, 3.0));

	// Configure g-buffer framebuffer
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);

	// 1280 for screen width & 960 for screen height
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 960, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 960, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 960, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// Tell OpenGL which of the color buffers associated with GBuffer we’d like to render to with glDrawBuffers
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

	srand(13);
	for (unsigned int i = 0; i < NR_LIGHTS; i++)
	{
		// Calculate slightly random offsets
		float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
		float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
		float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
		lightPositions.push_back(vec3(xPos, yPos, zPos));

		// Calculate random color
		float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
		float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
		float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
		lightColors.push_back(vec3(rColor, gColor, bColor));
	}

	// Use the lighting pass deferred shading shaders
	glUseProgram(deferredShadings[1]->shaderProgram);
	glUniform1i(glGetUniformLocation(deferredShadings[1]->shaderProgram, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(deferredShadings[1]->shaderProgram, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(deferredShadings[1]->shaderProgram, "gAlbedoSpec"), 2);
}

void DeferredShading::RenderDeferredShading()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Geometry pass: render scene's geometry/color data into gbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mat4 projection = perspective(radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 100.0f);
	mat4 view = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);
	mat4 model = mat4(1.0f);

	glUseProgram(deferredShadings[0]->shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(deferredShadings[0]->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(projection));

	glUniformMatrix4fv(glGetUniformLocation(deferredShadings[0]->shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

	for (unsigned int i = 0; i < objectPositions.size(); i++)
	{
		model = mat4(1.0f);
		model = translate(model, objectPositions[i]);
		model = scale(model, vec3(0.5f));

		glUniformMatrix4fv(glGetUniformLocation(deferredShadings[0]->shaderProgram, "model"), 1, GL_FALSE, 
			value_ptr(model));

		backpack->DrawModel(deferredShadings[0]);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(deferredShadings[1]->shaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

	// Send light relevant uniforms
	for (unsigned int i = 0; i < lightPositions.size(); i++)
	{
		glUniform3fv(glGetUniformLocation(deferredShadings[1]->shaderProgram, 
		("lights[" + to_string(i) + "].Position").c_str()), 1, value_ptr(lightPositions[i]));

		glUniform3fv(glGetUniformLocation(deferredShadings[1]->shaderProgram, 
			("lights[" + to_string(i) + "].Color").c_str()), 1, value_ptr(lightColors[i]));

		// Update attenuation parameters and calculate radius
		glUniform1f(glGetUniformLocation(deferredShadings[1]->shaderProgram,
			("lights[" + to_string(i) + "].Linear").c_str()), linear);

		glUniform1f(glGetUniformLocation(deferredShadings[1]->shaderProgram,
			("lights[" + to_string(i) + "].Quadratic").c_str()), quadratic);

		// Then calculate radius of light volume/sphere (Deferred Shading Part 2)
		const float maxBrightness = fmaxf(fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);

		float radius = (-linear + sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / 
			(2.0f * quadratic);
		
		glUniform1f(glGetUniformLocation(deferredShadings[1]->shaderProgram, 
			("lights[" + to_string(i) + "].Radius").c_str()), radius);
	}

	glUniform3fv(glGetUniformLocation(deferredShadings[1]->shaderProgram, "viewPos"), 1, value_ptr(Camera::cameraPosition));
	
	// Render quad after all the deferred shading shader uniforms are found and set
	RenderQuad();

	// Copy content of geometry's depth buffer to default framebuffer's depth buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer

	/* Blit to default framebuffer. This may or may not work as the internal formats of both the FBO and default framebuffer 
	have to match since the internal formats are implementation defined. This works on all of my systems, but if it doesn't 
	on yours you'll likely have to write to the depth buffer in another shader stage (or somehow see to match the default 
	framebuffer's internal format with the FBO's internal format) */
	glBlitFramebuffer(0, 0, 1280, 960, 0, 0, 1280, 960, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Render lights on top of scene
	glUseProgram(deferredShadings[2]->shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(deferredShadings[2]->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(projection));

	glUniformMatrix4fv(glGetUniformLocation(deferredShadings[2]->shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

	for (unsigned int i = 0; i < lightPositions.size(); i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPositions[i]);
		model = glm::scale(model, vec3(0.125f));

		glUniformMatrix4fv(glGetUniformLocation(deferredShadings[2]->shaderProgram, "model"), 1, GL_FALSE, 
			value_ptr(model));

		glUniform3fv(glGetUniformLocation(deferredShadings[2]->shaderProgram, "lightColor"), 1, value_ptr(lightColors[i]));
		
		RenderCube();
	}
}

void DeferredShading::RenderQuad()
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

		// setup plane VAO
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

void DeferredShading::RenderCube()
{
	if (cubeVAO == 0)
	{
		array<float, 288> vertices = 
		{
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
