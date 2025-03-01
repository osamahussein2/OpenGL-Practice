#include "DiffuseIrradiance.h"
#include "Camera.h"

DiffuseIrradiance* DiffuseIrradiance::objInstance = 0;

/* IBL, or image based lighting, is a collection of techniques to light objects, not by direct analytical lights, but by 
treating the surrounding environment as one big light source. This is generally accomplished by manipulating a cubemap 
environment map (taken from the real world or generated from a 3D scene) such that we can directly use it in our lighting 
equations: treating each cubemap texel as a light emitter. This way we can effectively capture an environment’s global 
lighting and general feel, giving objects a better sense of belonging in their environment */

DiffuseIrradiance::DiffuseIrradiance() : irradianceShaders{ new ShaderProgram(), new ShaderProgram(), new ShaderProgram(),
new ShaderProgram() }, sphereVAO(0), cubeVAO(0), nrColumns(0), nrRows(0), spacing(0), projection(mat4(0.0f)), 
view(mat4(0.0f)), model(mat4(0.0f)), indexCount(0)
{
}

DiffuseIrradiance::~DiffuseIrradiance()
{
}

DiffuseIrradiance* DiffuseIrradiance::Instance()
{
	if (objInstance == 0)
	{
		objInstance = new DiffuseIrradiance();
		
		return objInstance;
	}

	return objInstance;
}

void DiffuseIrradiance::InitializeDiffuseIrradiance()
{
	glEnable(GL_DEPTH_TEST);

	/* The xyww trick here that ensures the depth value of the rendered cube fragments always end up at 1.0, the maximum 
	depth value. We would need to change the depth comparison function to GL_LEQUAL */
	glDepthFunc(GL_LEQUAL);

	// PBR shader
	irradianceShaders[0]->InitializeShaderProgram(new VertexShaderLoader("PBRLightingVertexShader.glsl"),
		new FragmentShaderLoader("PBRLightingFragmentShader.glsl"));

	// Equirectangular to cubemap shader
	irradianceShaders[1]->InitializeShaderProgram(new VertexShaderLoader("DiffuseIrradianceCubemapVertexShader.glsl"),
		new FragmentShaderLoader("DiffuseIrradianceEquirectangularToCubemapFragmentShader.glsl"));

	// Background shader
	irradianceShaders[2]->InitializeShaderProgram(new VertexShaderLoader("DiffuseIrradianceBackgroundVertexShader.glsl"),
		new FragmentShaderLoader("DiffuseIrradianceBackgroundFragmentShader.glsl"));

	// Irradiance convolution shader (Diffuse Irradiance Part 2 only)
	irradianceShaders[3]->InitializeShaderProgram(new VertexShaderLoader("DiffuseIrradianceCubemapVertexShader.glsl"),
		new FragmentShaderLoader("DiffuseIrradianceConvolutionFragmentShader.glsl"));

	glUseProgram(irradianceShaders[0]->shaderProgram);

	glUniform1i(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "irradianceMap"), 0);
	glUniform3f(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "albedo"), 0.5f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "ambientOcclusion"), 1.0f);

	glUseProgram(irradianceShaders[2]->shaderProgram);

	glUniform1i(glGetUniformLocation(irradianceShaders[2]->shaderProgram, "environmentMap"), 0);

	lightPositions = 
	{
		vec3(-10.0f,  10.0f, 10.0f),
		vec3(10.0f,  10.0f, 10.0f),
		vec3(-10.0f, -10.0f, 10.0f),
		vec3(10.0f, -10.0f, 10.0f),
	};

	lightColors = 
	{
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f)
	};

	nrRows = 7;
	nrColumns = 7;
	spacing = 2.5;

	// Setup framebuffer for PBR
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	stbi_set_flip_vertically_on_load(true);

	data = stbi_loadf("Textures/HDR/newport_loft.hdr", &width, &height, &nrComponents, 0);

	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}

	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
	}

	// PBR: setup cubemap to render to and attach to framebuffer
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	captureProjection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	captureViews =
	{
		lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
		lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
		lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
		lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f))
	};

	// PBR: convert HDR equirectangular environment map to cubemap equivalent
	glUseProgram(irradianceShaders[1]->shaderProgram);

	glUniform1i(glGetUniformLocation(irradianceShaders[1]->shaderProgram, "equirectangularMap"), 0);

	glUniformMatrix4fv(glGetUniformLocation(irradianceShaders[1]->shaderProgram, "projection"), 1, GL_FALSE,
		value_ptr(captureProjection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 512, 512); // Configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(irradianceShaders[1]->shaderProgram, "view"), 1, GL_FALSE, 
			value_ptr(captureViews[i]));

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderCube();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// PBR: create an irradiance cubemap, and re-scale capture FBO to irradiance scale (Diffuse Irradiance Part 2)

	/* As the irradiance map averages all surrounding radiance uniformly it doesn’t have a lot of high frequency details, so 
	we can store the map at a low resolution (32x32) and let OpenGL’s linear filtering do most of the work */
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// PBR: solve diffuse integral by convolution to create an irradiance cube map
	glUseProgram(irradianceShaders[3]->shaderProgram);

	glUniform1i(glGetUniformLocation(irradianceShaders[3]->shaderProgram, "environmentMap"), 0);

	glUniformMatrix4fv(glGetUniformLocation(irradianceShaders[3]->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(captureProjection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glViewport(0, 0, 32, 32); // Configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(irradianceShaders[3]->shaderProgram, "view"), 1, GL_FALSE, 
			value_ptr(captureViews[i]));

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderCube();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Initialize static shader uniforms before rendering
	projection = perspective(glm::radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 100.0f);
	
	glUseProgram(irradianceShaders[0]->shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(projection));
	
	glUseProgram(irradianceShaders[2]->shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(irradianceShaders[2]->shaderProgram, "projection"), 1, GL_FALSE,
		value_ptr(projection));
}

void DiffuseIrradiance::RenderDiffuseIrradiance()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render scene, supplying the convoluted irradiance map to the final shader
	glUseProgram(irradianceShaders[0]->shaderProgram);

	view = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);

	glUniformMatrix4fv(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "view"), 1, GL_FALSE,
		value_ptr(view));

	glUniform3fv(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "cameraPosition"), 1,
		value_ptr(Camera::cameraPosition));

	// Bind pre-computed IBL data
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

	// render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
	model = glm::mat4(1.0f);

	for (int row = 0; row < nrRows; ++row)
	{
		glUniform1f(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "metallic"), float(row / nrRows));

		for (int col = 0; col < nrColumns; ++col)
		{
			// Clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
			// on direct lighting.
			glUniform1f(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "roughness"),
				clamp(float(col / nrColumns), 0.05f, 1.0f));

			model = mat4(1.0f);
			model = translate(model, vec3((float)(col - (nrColumns / 2)) * spacing,
				(float)(row - (nrRows / 2)) * spacing, -2.0f));

			glUniformMatrix4fv(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "model"), 1, GL_FALSE,
				value_ptr(model));

			glUniformMatrix3fv(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "normalMatrix"), 1, GL_TRUE,
				value_ptr(inverse(mat3(model))));

			RenderSphere();
		}
	}

	/* Render light source(simply re-render sphere at light positions), which will make their positions obvious and keeps
	the codeprint small */

	for (unsigned int i = 0; i < lightPositions.size(); ++i)
	{
		vec3 newPos = lightPositions[i] + vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
		newPos = lightPositions[i];

		glUniform3f(glGetUniformLocation(irradianceShaders[0]->shaderProgram,
			("lightPositions[" + to_string(i) + "]").c_str()), newPos.x, newPos.y, newPos.z);

		glUniform3f(glGetUniformLocation(irradianceShaders[0]->shaderProgram,
			("lightColors[" + to_string(i) + "]").c_str()), lightColors[i].x, lightColors[i].y, lightColors[i].z);

		model = mat4(1.0f);
		model = translate(model, newPos);
		model = scale(model, vec3(0.5f));

		glUniformMatrix4fv(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "model"), 1, GL_FALSE,
			value_ptr(model));

		glUniformMatrix3fv(glGetUniformLocation(irradianceShaders[0]->shaderProgram, "normalMatrix"), 1, GL_TRUE,
			value_ptr(inverse(mat3(model))));

		RenderSphere();
	}

	// Render skybox (render as last to prevent overdraw)
	glUseProgram(irradianceShaders[2]->shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(irradianceShaders[2]->shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map

	RenderCube();
}

void DiffuseIrradiance::RenderSphere()
{
	if (sphereVAO == 0)
	{
		glGenVertexArrays(1, &sphereVAO);

		unsigned int vbo, ebo;
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		vector<vec3> positions;
		vector<vec2> uv;
		vector<vec3> normals;
		vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;

		const float PI = 3.14159265359f;

		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;

				float xPos = cos(xSegment * 2.0f * PI) * sin(ySegment * PI);
				float yPos = cos(ySegment * PI);
				float zPos = sin(xSegment * 2.0f * PI) * sin(ySegment * PI);

				positions.push_back(vec3(xPos, yPos, zPos));
				uv.push_back(vec2(xSegment, ySegment));
				normals.push_back(vec3(xPos, yPos, zPos));
			}
		}

		bool oddRowNumber = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRowNumber)
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}

			oddRowNumber = !oddRowNumber;
		}

		indexCount = indices.size();

		vector<float> data;

		for (unsigned int i = 0; i < positions.size(); ++i)
		{
			data.push_back(positions[i].x);
			data.push_back(positions[i].y);
			data.push_back(positions[i].z);

			if (normals.size() > 0)
			{
				data.push_back(normals[i].x);
				data.push_back(normals[i].y);
				data.push_back(normals[i].z);
			}

			if (uv.size() > 0)
			{
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
		}

		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		unsigned int stride = sizeof(vec3) + sizeof(vec2) + sizeof(vec3);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	}

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

void DiffuseIrradiance::RenderCube()
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