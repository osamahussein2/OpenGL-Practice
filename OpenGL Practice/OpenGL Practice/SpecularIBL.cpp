#include "SpecularIBL.h"
#include "Camera.h"

SpecularIBL* SpecularIBL::specularIBLinstance = 0;

/* For the same (performance) reasons as the irradiance convolution, we can’t solve the specular part of the integral in real 
time and expect a reasonable performance. So preferably we’d pre-compute this integral to get something like a specular IBL 
map, sample this map with the fragment’s normal, and be done with it */

/* For increasing roughness levels, the environment map is convoluted with more scattered sample vectors, creating blurrier 
reflections. For each roughness level we convolute, we store the sequentially blurrier results in the pre-filtered map’s 
mipmap levels */

/* The second part of the split sum equation equals the BRDF part of the specular integral. If we pretend the incoming 
radiance is completely white for every direction (thus L(p, x) = 1.0) we can pre-calculate the BRDF’s response given an input 
roughness and an input angle between the normal n and light direction ωi, or n · ωi. Epic Games stores the pre-computed 
BRDF’s response to each normal and light direction combination on varying roughness values in a 2D lookup texture (LUT)
known as the BRDF integration map */

/* Monte Carlo integration revolves mostly around a combination of statistics and probability theory. Monte Carlo helps us 
in discretely solving the problem of figuring out some statistic or value of a population without having to take all of the
population into consideration */

/* A different approach is to pick a much smaller completely random (unbiased) subset of this population, measure their 
height, and average the result. This population could be as small as a 100 people. While not as accurate as the exact 
answer, you’ll get an answer that is relatively close to the ground truth. This is known as the law of large numbers. The 
idea is that if you measure a smaller set of size N of truly random samples from the total population, the result will be 
relatively close to the true answer and gets closer as the number of samples N increases */

/* Monte Carlo integration builds on this law of large numbers and takes the same approach in solving an integral. Rather 
than solving an integral for all possible (theoretically infinite) sample values x, simply generate N sample values randomly 
picked from the total population and average */

/* To solve the integral, we take N random samples over the population a to b, add them together, and divide by the total 
number of samples to average them. The pd f stands for the probability density function that tells us the probability a 
specific sample occurs over the total sample set */

SpecularIBL::SpecularIBL() : specularIBLshaders{new ShaderProgram(), new ShaderProgram(), new ShaderProgram(),
new ShaderProgram(), new ShaderProgram(), new ShaderProgram() }, cubeVAO(0), cubeVBO(0), sphereVAO(0), indexCount(0),
quadVAO(0), quadVBO(0), projection(mat4(0.0f)), brdfLUTTexture(0), maxMipLevels(0), prefilterMap(0), irradianceMap(0),
captureViews{ mat4(0.0f), mat4(0.0f),mat4(0.0f),mat4(0.0f),mat4(0.0f),mat4(0.0f) }, captureFBO(0), 
captureProjection(mat4(0.0f)), captureRBO(0), data(0), envCubemap(0), hdrTexture(0), lightColors{ vec3(0.0f), vec3(0.0f),
vec3(0.0f), vec3(0.0f) }, lightPositions{ vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f) }, nrColumns(0), nrRows(0), 
spacing(0), ironAlbedoMap(0), ironNormalMap(0), ironMetallicMap(0), ironRoughnessMap(0), ironAOMap(0), goldAlbedoMap(0),
goldNormalMap(0), goldMetallicMap(0), goldRoughnessMap(0), goldAOMap(0), grassAlbedoMap(0), grassNormalMap(0), 
grassMetallicMap(0), grassRoughnessMap(0), grassAOMap(0), plasticAlbedoMap(0), plasticNormalMap(0), plasticMetallicMap(0),
plasticRoughnessMap(0), plasticAOMap(0), wallAlbedoMap(0), wallNormalMap(0), wallMetallicMap(0), wallRoughnessMap(0), 
wallAOMap(0)
{
}

SpecularIBL::~SpecularIBL()
{
	for (int i = 0; i < specularIBLshaders.size(); i++)
	{
		delete specularIBLshaders[i];
	}

	array<mat4, 2> mat4Variables =
	{
		captureProjection, projection
	};

	for (unsigned int i = 0; i < mat4Variables.size(); i++)
	{
		mat4Variables[i] = mat4(0.0f);
	}

	for (unsigned int i = 0; i < captureViews.size(); i++)
	{
		captureViews[i] = mat4(0.0f);
	}

	array<array<vec3, 4>, 2> lightsInfo =
	{
		lightColors, lightPositions
	};

	for (unsigned int i = 0; i < lightsInfo.size(); i++)
	{
		lightsInfo[i] = { vec3(0.0f), vec3(0.0f) };
	}

	data = nullptr;

	array<unsigned int, 42> integerVariables =
	{
		(unsigned int)nrRows, (unsigned int)nrColumns, (unsigned int)spacing, captureFBO, captureRBO, hdrTexture, 
		envCubemap, irradianceMap, prefilterMap, maxMipLevels, brdfLUTTexture, sphereVAO, indexCount, cubeVAO, cubeVBO, 
		quadVAO, quadVBO, ironAlbedoMap, ironNormalMap, ironMetallicMap, ironRoughnessMap, ironAOMap, goldAlbedoMap, 
		goldNormalMap, goldMetallicMap, goldRoughnessMap, goldAOMap, grassAlbedoMap, grassNormalMap, grassMetallicMap, 
		grassRoughnessMap, grassAOMap, plasticAlbedoMap, plasticNormalMap, plasticMetallicMap, plasticRoughnessMap, 
		plasticAOMap, wallAlbedoMap, wallNormalMap, wallMetallicMap, wallRoughnessMap, wallAOMap
	};

	for (unsigned int i = 0; i < integerVariables.size(); i++)
	{
		integerVariables[i] = NULL;
	}
}

SpecularIBL* SpecularIBL::Instance()
{
	if (specularIBLinstance == 0)
	{
		specularIBLinstance = new SpecularIBL();

		return specularIBLinstance;
	}

	return specularIBLinstance;
}

void SpecularIBL::InitializeSpecularIBL()
{
	glEnable(GL_DEPTH_TEST);

	/* The xyww trick here that ensures the depth value of the rendered cube fragments always end up at 1.0, the maximum
	depth value. We would need to change the depth comparison function to GL_LEQUAL */
	glDepthFunc(GL_LEQUAL);

	// OpenGL gives us the option to properly filter across cubemap faces by enabling GL_TEXTURE_CUBE_MAP_SEAMLESS
	// Enable seamless cubemap sampling for lower mip levels in the pre-filter map
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// PBR shader
	specularIBLshaders[0]->InitializeShaderProgram(new VertexShaderLoader("PBRLightingVertexShader.glsl"), 
		new FragmentShaderLoader("PBRLightingFragmentShader.glsl"));

	// Equirectangular to cubemap shader
	specularIBLshaders[1]->InitializeShaderProgram(new VertexShaderLoader("DiffuseIrradianceCubemapVertexShader.glsl"),
		new FragmentShaderLoader("DiffuseIrradianceEquirectangularToCubemapFragmentShader.glsl"));

	// Irradiance shader
	specularIBLshaders[2]->InitializeShaderProgram(new VertexShaderLoader("DiffuseIrradianceCubemapVertexShader.glsl"),
		new FragmentShaderLoader("DiffuseIrradianceConvolutionFragmentShader.glsl"));

	// Prefilter shader
	specularIBLshaders[3]->InitializeShaderProgram(new VertexShaderLoader("DiffuseIrradianceCubemapVertexShader.glsl"),
		new FragmentShaderLoader("SpecularIBL_Prefilter_FragmentShader.glsl"));

	// BRDF shader
	specularIBLshaders[4]->InitializeShaderProgram(new VertexShaderLoader("SpecularIBL_BRDF_VertexShader.glsl"),
		new FragmentShaderLoader("SpecularIBL_BRDF_FragmentShader.glsl"));

	// Background shader
	specularIBLshaders[5]->InitializeShaderProgram(new VertexShaderLoader("DiffuseIrradianceBackgroundVertexShader.glsl"),
		new FragmentShaderLoader("DiffuseIrradianceBackgroundFragmentShader.glsl"));

	glUseProgram(specularIBLshaders[0]->shaderProgram);
	glUniform1i(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "irradianceMap"), 0);
	glUniform1i(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "prefilterMap"), 1);
	glUniform1i(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "brdfLUT"), 2);

	// Specular IBL Part 2 textures
	glUniform1i(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "albedoMap"), 3);
	glUniform1i(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "normalMap"), 4);
	glUniform1i(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "metallicMap"), 5);
	glUniform1i(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "roughnessMap"), 6);
	glUniform1i(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "aoMap"), 7);

	// Specular IBL Part 1 material color
	//glUniform3f(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "albedo"), 0.5f, 0.0f, 0.0f);
	//glUniform1f(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "ambientOcclusion"), 1.0f);

	glUseProgram(specularIBLshaders[5]->shaderProgram);
	glUniform1i(glGetUniformLocation(specularIBLshaders[5]->shaderProgram, "environmentMap"), 0);

	// Load PBR material textures (Specular IBL Part 2 textures)
	
	// rusted iron
	ironAlbedoMap = LoadTexture("Textures/PBR/rusted_iron/albedo.png");
	ironNormalMap = LoadTexture("Textures/PBR/rusted_iron/normal.png");
	ironMetallicMap = LoadTexture("Textures/PBR/rusted_iron/metallic.png");
	ironRoughnessMap = LoadTexture("Textures/PBR/rusted_iron/roughness.png");
	ironAOMap = LoadTexture("Textures/PBR/rusted_iron/ao.png");

	// gold
	goldAlbedoMap = LoadTexture("Textures/PBR/gold/albedo.png");
	goldNormalMap = LoadTexture("Textures/PBR/gold/normal.png");
	goldMetallicMap = LoadTexture("Textures/PBR/gold/metallic.png");
	goldRoughnessMap = LoadTexture("Textures/PBR/gold/roughness.png");
	goldAOMap = LoadTexture("Textures/PBR/gold/ao.png");

	// grass
	grassAlbedoMap = LoadTexture("Textures/PBR/grass/albedo.png");
	grassNormalMap = LoadTexture("Textures/PBR/grass/normal.png");
	grassMetallicMap = LoadTexture("Textures/PBR/grass/metallic.png");
	grassRoughnessMap = LoadTexture("Textures/PBR/grass/roughness.png");
	grassAOMap = LoadTexture("Textures/PBR/grass/ao.png");

	// plastic
	plasticAlbedoMap = LoadTexture("Textures/PBR/plastic/albedo.png");
	plasticNormalMap = LoadTexture("Textures/PBR/plastic/normal.png");
	plasticMetallicMap = LoadTexture("Textures/PBR/plastic/metallic.png");
	plasticRoughnessMap = LoadTexture("Textures/PBR/plastic/roughness.png");
	plasticAOMap = LoadTexture("Textures/PBR/plastic/ao.png");

	// wall
	wallAlbedoMap = LoadTexture("Textures/PBR/wall/albedo.png");
	wallNormalMap = LoadTexture("Textures/PBR/wall/normal.png");
	wallMetallicMap = LoadTexture("Textures/PBR/wall/metallic.png");
	wallRoughnessMap = LoadTexture("Textures/PBR/wall/roughness.png");
	wallAOMap = LoadTexture("Textures/PBR/wall/ao.png");

	lightPositions = 
	{
		glm::vec3(-10.0f,  10.0f, 10.0f),
		glm::vec3(10.0f,  10.0f, 10.0f),
		glm::vec3(-10.0f, -10.0f, 10.0f),
		glm::vec3(10.0f, -10.0f, 10.0f),
	};
	
	lightColors = 
	{
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f)
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

	// Load the HDR environment map for PBR
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrComponents;
	data = stbi_loadf("Textures/HDR/newport_loft.hdr", &width, &height, &nrComponents, 0);

	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);

		// Specify the texture's data value to be float
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

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

	// Setup cubemap to render to and attach to framebuffer for PBR
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Enable trilinear filtering on the environment map you want to sample its mip levels from

	// Enable pre-filter mipmap sampling (combatting visible dots artifact)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set up projection and view matrices for capturing data onto the 6 cubemap face directions for PBR
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

	// Convert HDR equirectangular environment map to cubemap equivalent for PBR
	glUseProgram(specularIBLshaders[1]->shaderProgram);
	glUniform1i(glGetUniformLocation(specularIBLshaders[1]->shaderProgram, "equirectangularMap"), 0);

	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[1]->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(captureProjection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	// Configure the viewport to the capture dimensions
	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[1]->shaderProgram, "view"), 1, GL_FALSE, 
			value_ptr(captureViews[i]));

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Then let OpenGL generate mipmaps from first mip face
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// Create an irradiance cubemap, and re-scale capture FBO to irradiance scale for PBR
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

	// Solve diffuse integral by convolution to create an irradiance cubemap for PBR
	glUseProgram(specularIBLshaders[2]->shaderProgram);
	glUniform1i(glGetUniformLocation(specularIBLshaders[2]->shaderProgram, "environmentMap"), 0);

	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[2]->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(captureProjection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glViewport(0, 0, 32, 32); // Configure the viewport to the capture dimensions
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[2]->shaderProgram, "view"), 1, GL_FALSE, 
			value_ptr(captureViews[i]));

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale for PBR
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Set the minification filter to mipmap linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate mipmaps for the cubemap so OpenGL automatically allocates the required memory
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// Run a quasi monte-carlo simulation on the environment lighting to create a prefilter cubemap for PBR
	glUseProgram(specularIBLshaders[3]->shaderProgram);
	glUniform1i(glGetUniformLocation(specularIBLshaders[3]->shaderProgram, "environmentMap"), 0);

	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[3]->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(captureProjection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	maxMipLevels = 5;

	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// Reisze framebuffer according to mip-level size
		unsigned int mipWidth = static_cast<unsigned int>(128 * pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(128 * pow(0.5, mip));

		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);

		glUniform1f(glGetUniformLocation(specularIBLshaders[3]->shaderProgram, "roughness"), roughness);

		for (unsigned int i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[3]->shaderProgram, "view"), 1, GL_FALSE, 
				value_ptr(captureViews[i]));

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
				prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Generate a 2D LUT from the BRDF equations used for PBR
	glGenTextures(1, &brdfLUTTexture);

	// Pre-allocate enough memory for the LUT texture
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);

	// Set wrapping mode to GL_CLAMP_TO_EDGE for the LUT texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Then re-configure capture framebuffer object and render screen-space quad with BRDF shader
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	glViewport(0, 0, 512, 512);
	glUseProgram(specularIBLshaders[4]->shaderProgram);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// initialize static shader uniforms before rendering
	projection = perspective(radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 100.0f);

	glUseProgram(specularIBLshaders[0]->shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(projection));

	glUseProgram(specularIBLshaders[5]->shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[5]->shaderProgram, "projection"), 1, GL_FALSE,
		value_ptr(projection));
}

void SpecularIBL::RenderSpecularIBL()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render the scene that supplies the convoluted irradiance map to the final shader
	glUseProgram(specularIBLshaders[0]->shaderProgram);
	
	mat4 view = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);

	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));
	glUniform3fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "camPos"), 1, value_ptr(Camera::cameraPosition));

	// Bind pre-computed IBL data
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

	// Specular IBL Part 2 texture
	// rusted iron
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, ironAlbedoMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ironNormalMap);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, ironMetallicMap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, ironRoughnessMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, ironAOMap);

	mat4 model = mat4(1.0f);
	model = translate(model, vec3(-5.0, 0.0, 2.0));

	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "model"), 1, GL_FALSE,
		value_ptr(model));

	glUniformMatrix3fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "normalMatrix"), 1, GL_TRUE,
		value_ptr(inverse(glm::mat3(model))));

	RenderSphere();

	// gold
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, goldAlbedoMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, goldNormalMap);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, goldMetallicMap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, goldRoughnessMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, goldAOMap);

	model = mat4(1.0f);
	model = translate(model, vec3(-3.0, 0.0, 2.0));

	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "model"), 1, GL_FALSE,
		value_ptr(model));

	glUniformMatrix3fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "normalMatrix"), 1, GL_TRUE,
		value_ptr(inverse(glm::mat3(model))));

	RenderSphere();

	// grass
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, grassAlbedoMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, grassNormalMap);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, grassMetallicMap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, grassRoughnessMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, grassAOMap);

	model = mat4(1.0f);
	model = translate(model, vec3(-1.0, 0.0, 2.0));

	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "model"), 1, GL_FALSE,
		value_ptr(model));

	glUniformMatrix3fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "normalMatrix"), 1, GL_TRUE,
		value_ptr(inverse(glm::mat3(model))));

	RenderSphere();

	// plastic
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, plasticAlbedoMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, plasticNormalMap);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, plasticMetallicMap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, plasticRoughnessMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, plasticAOMap);

	model = mat4(1.0f);
	model = translate(model, vec3(1.0, 0.0, 2.0));

	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "model"), 1, GL_FALSE,
		value_ptr(model));

	glUniformMatrix3fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "normalMatrix"), 1, GL_TRUE,
		value_ptr(inverse(glm::mat3(model))));

	RenderSphere();

	// wall
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, wallAlbedoMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, wallNormalMap);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, wallMetallicMap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, wallRoughnessMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, wallAOMap);

	model = mat4(1.0f);
	model = translate(model, vec3(3.0, 0.0, 2.0));

	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "model"), 1, GL_FALSE,
		value_ptr(model));

	glUniformMatrix3fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "normalMatrix"), 1, GL_TRUE,
		value_ptr(inverse(glm::mat3(model))));

	RenderSphere();

	// Specular IBL Part 1 material color
	// Render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
	/*for (int row = 0; row < nrRows; ++row)
	{
		//glUniform1f(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "metallic"), (float)row / (float)nrRows);

		for (int col = 0; col < nrColumns; ++col)
		{
			// we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
			// on direct lighting.
			//glUniform1f(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "roughness"), 
				//clamp(float(col / nrColumns), 0.05f, 1.0f));

			model = mat4(1.0f);
			model = translate(model, vec3(
				(float)(col - (nrColumns / 2)) * spacing,
				(float)(row - (nrRows / 2)) * spacing,
				-2.0f
			));

			glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "model"), 1, GL_FALSE, 
				value_ptr(model));

			glUniformMatrix3fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "normalMatrix"), 1, GL_TRUE, 
				value_ptr(inverse(glm::mat3(model))));

			RenderSphere();
		}
	}*/

	/* Render light source (simply re-render sphere at light positions) this looks a bit off as we use the same shader, but 
	it'll make their positions obvious and keeps the codeprint small */
	for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
	{
		glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
		newPos = lightPositions[i];

		glUniform3fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, 
			("lightPositions[" + to_string(i) + "]").c_str()), 1, value_ptr(newPos));
		
		glUniform3fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram,
			("lightColors[" + to_string(i) + "]").c_str()), 1, value_ptr(lightColors[i]));

		model = mat4(1.0f);
		model = translate(model, newPos);
		model = scale(model, vec3(0.5f));

		glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "model"), 1, GL_FALSE,
			value_ptr(model));

		glUniformMatrix3fv(glGetUniformLocation(specularIBLshaders[0]->shaderProgram, "normalMatrix"), 1, GL_TRUE, 
			value_ptr(inverse(mat3(model))));
		
		RenderSphere();
	}

	// render skybox (render as last to prevent overdraw)
	glUseProgram(specularIBLshaders[5]->shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(specularIBLshaders[5]->shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	//glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
	//glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map

	RenderCube();

	// Render BRDF map to screen
	//glUseProgram(specularIBLshaders[4]->shaderProgram);
	//RenderQuad();
}

unsigned int SpecularIBL::LoadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1) format = GL_RED;
		else if (nrComponents == 3) format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void SpecularIBL::RenderSphere()
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

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
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
			oddRow = !oddRow;
		}

		indexCount = static_cast<unsigned int>(indices.size());

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
		unsigned int stride = (3 + 2 + 3) * sizeof(float);
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

void SpecularIBL::RenderCube()
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

void SpecularIBL::RenderQuad()
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
