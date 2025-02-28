#include "PBRLighting.h"
#include "Camera.h"

PBRLighting* PBRLighting::pbrLightingInstance = NULL;

/* PBR (physically based rendering) is a collection of render techniques that are more or less based on the same underlying 
theory that more closely matches that of the physical world. As physically based rendering aims to mimic light in a physically 
plausible way, it generally looks more realistic compared to our original lighting algorithms like Phong and Blinn-Phong */

/* Physically based rendering is still nonetheless an approximation of reality (based on the principles of physics) which is 
why it’s not called physical shading, but physically based shading.

For a PBR lighting model to be considered physically based, it has to satisfy the following 3 conditions:

	1. Be based on the microfacet surface model
	2. Be energy conserving
	3. Use a physically based BRDF 
*/

/* All the PBR techniques are based on the theory of microfacets. The theory describes that any surface at a microscopic 
scale can be described by tiny little perfectly reflective mirrors called microfacets */

PBRLighting::PBRLighting() : pbrLightingShader(new ShaderProgram()), /*lightColors{vec3(0.0f), vec3(0.0f), vec3(0.0f),
vec3(0.0f) }, lightPositions{vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)},*/ lightPositions{vec3(0.0f)}, 
lightColors{vec3(0.0f)}, nrColumns(0), nrRows(0), spacing(0), albedo(0), metallic(0), roughness(0), normal(0), 
ambientOcclusion(0), projectionMatrix(mat4(0.0f)), viewMatrix(mat4(0.0f)), modelMatrix(mat4(0.0f)), sphereVAO(0), 
indexCount(0)
{
}

PBRLighting::~PBRLighting()
{
	array<array<vec3, 1>, 2> lights = { lightColors, lightPositions };

	for (unsigned int i = 0; i < lights.size(); i++)
	{
		lights[i] = { vec3(0.0f) };
	}

	delete pbrLightingShader;

	array<int, 10> intVariables = { nrColumns, nrRows, static_cast<int>(spacing), sphereVAO, indexCount, albedo, metallic, 
		roughness, normal, ambientOcclusion };

	for (unsigned int i = 0; i < intVariables.size(); i++)
	{
		intVariables[i] = NULL;
	}

	array<mat4, 3> shaderMatrices = { projectionMatrix, viewMatrix, modelMatrix };

	for (unsigned int i = 0; i < shaderMatrices.size(); i++)
	{
		shaderMatrices[i] = mat4(NULL);
	}

	pbrLightingInstance = NULL;
}

PBRLighting* PBRLighting::Instance()
{
	if (pbrLightingInstance == NULL)
	{
		pbrLightingInstance = new PBRLighting();

		return pbrLightingInstance;
	}

	return pbrLightingInstance;
}

void PBRLighting::InitializePBRLighting()
{
	glEnable(GL_DEPTH_TEST);

	pbrLightingShader->InitializeShaderProgram(new VertexShaderLoader("PBRLightingVertexShader.glsl"),
		new FragmentShaderLoader("PBRLightingFragmentShader.glsl"));
	
	glUseProgram(pbrLightingShader->shaderProgram);

	/* PBR Lighting Part 1
	glUniform3f(glGetUniformLocation(pbrLightingShader->shaderProgram, "albedo"), 0.5f, 0.0f, 0.0f);
	/glUniform1f(glGetUniformLocation(pbrLightingShader->shaderProgram, "ambientOcclusion"), 1.0f); */

	// PBR Lighting Part 2
	glUniform1i(glGetUniformLocation(pbrLightingShader->shaderProgram, "albedoMap"), 0);
	glUniform1i(glGetUniformLocation(pbrLightingShader->shaderProgram, "normalMap"), 1);
	glUniform1i(glGetUniformLocation(pbrLightingShader->shaderProgram, "metallicMap"), 2);
	glUniform1i(glGetUniformLocation(pbrLightingShader->shaderProgram, "roughnessMap"), 3);
	glUniform1i(glGetUniformLocation(pbrLightingShader->shaderProgram, "aoMap"), 4);

	albedo = LoadTexture("Textures/PBR/rusted_iron/albedo.png");
	normal = LoadTexture("Textures/PBR/rusted_iron/normal.png");
	metallic = LoadTexture("Textures/PBR/rusted_iron/metallic.png");
	roughness = LoadTexture("Textures/PBR/rusted_iron/roughness.png");
	ambientOcclusion = LoadTexture("Textures/PBR/rusted_iron/ao.png");

	// PBR Lighting Part 1
	/*lightPositions =
	{
		vec3(-10.0f,  10.0f, 10.0f), vec3(10.0f,  10.0f, 10.0f),
		vec3(-10.0f, -10.0f, 10.0f), vec3(10.0f, -10.0f, 10.0f)
	};

	lightColors = 
	{
		vec3(300.0f, 300.0f, 300.0f), vec3(300.0f, 300.0f, 300.0f), 
		vec3(300.0f, 300.0f, 300.0f), vec3(300.0f, 300.0f, 300.0f)
	};*/

	// PBR Lighting Part 2
	lightPositions = 
	{
		vec3(0.0f, 0.0f, 10.0f),
	};

	lightColors = 
	{
		vec3(150.0f, 150.0f, 150.0f),
	};

	nrRows = 7;
	nrColumns = 7;
	spacing = 2.5;

	// Initialize static shader projection matrix uniform before rendering
	projectionMatrix = perspective(radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 100.0f);
	
	glUniformMatrix4fv(glGetUniformLocation(pbrLightingShader->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(projectionMatrix));
}

void PBRLighting::RenderPBRLighting()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pbrLightingShader->shaderProgram);

	viewMatrix = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);
	glUniformMatrix4fv(glGetUniformLocation(pbrLightingShader->shaderProgram, "view"), 1, GL_FALSE, value_ptr(viewMatrix));
	
	glUniform3fv(glGetUniformLocation(pbrLightingShader->shaderProgram, "cameraPosition"), 1, 
		value_ptr(Camera::cameraPosition));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, albedo);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, metallic);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, roughness);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ambientOcclusion);

	// Render rows and column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
	modelMatrix = mat4(1.0f);

	for (int row = 0; row < nrRows; ++row)
	{
		// PBR Lighting Part 1
		//glUniform1f(glGetUniformLocation(pbrLightingShader->shaderProgram, "metallic"), float(row / nrRows));

		for (int col = 0; col < nrColumns; ++col)
		{
			/*Clamp the roughness between 0.05 to 1.0 as perfectly smooth surfaces(roughness of 0.0) tend to look a bit off 
			on direct lighting (PBR Lighting Part 1)
			glUniform1f(glGetUniformLocation(pbrLightingShader->shaderProgram, "roughness"), 
				clamp((float)col / (float)nrColumns, 0.05f, 1.0f));*/

			modelMatrix = mat4(1.0f);
			modelMatrix = translate(modelMatrix, glm::vec3(
				(col - (nrColumns / 2)) * spacing,
				(row - (nrRows / 2)) * spacing,
				0.0f
			));

			glUniformMatrix4fv(glGetUniformLocation(pbrLightingShader->shaderProgram, "model"), 1, GL_FALSE, 
				value_ptr(modelMatrix));

			glUniformMatrix3fv(glGetUniformLocation(pbrLightingShader->shaderProgram, "normalMatrix"), 1, 
				GL_TRUE, value_ptr(inverse(mat3(modelMatrix))));
			
			RenderSphere();
		}
		
		/* Render light source(simply re - render sphere at light positions) this looks a bit off as we use the same shader, 
		but it'll make their positions obvious and keeps the codeprint small */
		for (unsigned int i = 0; i < lightPositions.size(); ++i)
		{
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
			newPos = lightPositions[i];
			
			glUniform3fv(glGetUniformLocation(pbrLightingShader->shaderProgram, 
				("lightPositions[" + to_string(i) + "]").c_str()), 1, value_ptr(newPos));

			glUniform3fv(glGetUniformLocation(pbrLightingShader->shaderProgram, 
				("lightColors[" + to_string(i) + "]").c_str()), 1, value_ptr(lightColors[i]));

			modelMatrix = mat4(1.0f);
			modelMatrix = translate(modelMatrix, newPos);
			modelMatrix = scale(modelMatrix, glm::vec3(0.5f));

			glUniformMatrix4fv(glGetUniformLocation(pbrLightingShader->shaderProgram, "model"), 1, GL_FALSE, 
				value_ptr(modelMatrix));

			glUniformMatrix3fv(glGetUniformLocation(pbrLightingShader->shaderProgram, "normalMatrix"), 1, GL_TRUE,
				value_ptr(inverse(mat3(modelMatrix))));

			RenderSphere();
		}
	}
}

unsigned int PBRLighting::LoadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

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

void PBRLighting::RenderSphere()
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