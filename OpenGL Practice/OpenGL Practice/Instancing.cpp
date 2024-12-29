#include "Instancing.h"
#include "Camera.h"

/* Instancing is a technique where we draw many (equal mesh data) objects at once with a single render call, saving us all 
the CPU -> GPU communications each time we need to render an object. To render using instancing all we need to do is change 
the render calls glDrawArrays and glDrawElements to glDrawArraysInstanced and glDrawElementsInstanced respectively */

/* These instanced versions of the classic rendering functions take an extra parameter called the instance count that sets 
the number of instances we want to render. We sent all the required data to the GPU once, and then tell the GPU how it should 
draw all these instances with a single call. The GPU then renders all these instances without having to continually 
communicate with the CPU */

Instancing::Instancing() : instancingShaderProgram(new ShaderProgram()), modelShaderProgram(new ShaderProgram())
{
}

Instancing::~Instancing()
{
	glDeleteBuffers(1, &instanceVBO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &quadVAO);
}

void Instancing::SetInstancingOffsetPositions()
{
	// Instancing Part 1
	instancingShaderProgram->InitializeShaderProgram(new VertexShaderLoader("InstancingVertexShader.glsl"), new
		FragmentShaderLoader("InstancingFragmentShader.glsl"));

	int index = 0;
	float offset = 0.1f;
	for (int y = -10; y < 10; y += 2)
	{
		for (int x = -10; x < 10; x += 2)
		{
			// Create a set of 100 translation vectors that contains an offset vector for all positions in a 10x10 grid
			vec2 translation;
			translation.x = (float)x / 10.0f + offset;
			translation.y = (float)y / 10.0f + offset;
			translations[index++] = translation;
		}
	}

	/* Because an instanced array is a vertex attribute, just like the position and color variables, we need to store its
	content in a vertex buffer object and configure its attribute pointer */
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * 100, &translations[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Instancing::InitializeInstancingVertices()
{
	// Instancing Part 1
	quadVertices = 
	{
		// positions     // colors
		-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
		-0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

		-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
		 0.05f,  0.05f,  0.0f, 1.0f, 1.0f
	};

	glGenBuffers(1, &quadVBO);
	glGenVertexArrays(1, &quadVAO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

	// Then we also need to set its vertex attribute pointer and enable the vertex attribute
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* This function tells OpenGL when to update the content of a vertex attribute to the next element. Its first parameter 
	is the vertex attribute in question and the second parameter the attribute divisor. */

	/* By default, the attribute divisor is 0 which tells OpenGL to update the content of the vertex attribute each iteration 
	of the vertex shader. By setting this attribute to 1 we’re telling OpenGL that we want to update the content of the 
	vertex attribute when we start to render a new instance. By setting it to 2 we’d update the content every 2 instances 
	and so on. By setting the attribute divisor to 1 we’re effectively telling OpenGL that the vertex attribute at attribute 
	location 2 is an instanced array */
	glVertexAttribDivisor(2, 1);

}

void Instancing::SetTransformationMatrix()
{
	// Instancing Part 2
	instancingShaderProgram->InitializeShaderProgram(new VertexShaderLoader("InstancingVertexShader.glsl"), new
		FragmentShaderLoader("InstancingFragmentShader.glsl"));

	// Instancing Part 3 (since this has a model matrix but the instancing shaders have an instance matrix now)
	modelShaderProgram->InitializeShaderProgram(new VertexShaderLoader("ModelVertexShader.glsl"), new
		FragmentShaderLoader("ModelFragmentShader.glsl"));

	rock = new Model("Models/Rock/rock.obj");
	planet = new Model("Models/Planet/planet.obj");

	amount = 100000;

	modelMatrices = new glm::mat4[amount];

	srand(glfwGetTime()); // initialize random seed

	float radius = 150.0;
	float offset = 25.0f;

	for (unsigned int i = 0; i < amount; i++)
	{
		/* Transform the x and z position of the asteroid along a circle with a radius defined by radius and randomly 
		displace each asteroid a little around the circle by -offset and offset. We give the y displacement less of an impact
		to create a more flat asteroid ring. Then we apply scale and rotation transformations and store the resulting 
		transformation matrix in modelMatrices that is of size amount. Here we generate 1000 model matrices, 
		one per asteroid. */

		glm::mat4 model = glm::mat4(1.0f);

		// Displace along circle with radius by -offset and offset
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

		float y = displacement * 0.4f; // keep height of field smaller than x/z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

		float z = cos(angle) * radius + displacement;
		model = translate(model, vec3(x, y, z));

		// Scale between 0.05 and 0.25f
		float scale = (rand() % 20) / 100.0f + 0.05;
		model = glm::scale(model, vec3(scale));

		// Add random rotation around a (semi)random rotation axis
		float rotationAngle = (rand() % 360);
		model = rotate(model, rotationAngle, vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		modelMatrices[i] = model;
	}

}

void Instancing::SetInstancedArrays()
{
	// Instancing Part 3 (vertex buffer object)
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	for (unsigned int i = 0; i < rock->meshes.size(); i++)
	{
		unsigned int VAO = rock->meshes[i].VAO;
		glBindVertexArray(VAO);

		// Vertex attributes
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(1 * sizeof(glm::vec4)));

		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));

		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}

}

void Instancing::UseInstancingShaderProgram()
{
	/* In addition to generating the translations array, we’d also need to transfer the data to the vertex shader’s
	uniform array (Instancing Part 1) */
	//glUseProgram(instancingShaderProgram->shaderProgram);

	/* Transform the for-loop counter i to a string to dynamically create a location string for querying the uniform 
	location. For each item in the offsets uniform array we then set the corresponding translation vector */

	//glUniform2f(glGetUniformLocation(instancingShaderProgram->shaderProgram, "offsets[" + to_string(i) + "]")),
		//translations[i]); // LEARNING PURPOSES ONLY
		
	//glBindVertexArray(quadVAO);
	
	/* The parameters of glDrawArraysInstanced are exactly the same as glDrawArrays except the last parameter that sets the 
	number of instances we want to draw */
	//glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);

	//glBindVertexArray(0);

	// Instancing Part 2
	//glUseProgram(instancingShaderProgram->shaderProgram);

	// Configure transformation matrices (Instancing Part 3)
	glm::mat4 projectionMatrix = perspective(glm::radians(45.0f), float(800 / 600), 0.1f, 1000.0f);
	glm::mat4 viewMatrix = Camera::CameraLookAt();

	glUseProgram(instancingShaderProgram->shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(instancingShaderProgram->shaderProgram, "projectionMatrix"), 1,
		GL_FALSE, glm::value_ptr(projectionMatrix));

	glUniformMatrix4fv(glGetUniformLocation(instancingShaderProgram->shaderProgram, "viewMatrix"), 1,
		GL_FALSE, glm::value_ptr(viewMatrix));

	glUseProgram(modelShaderProgram->shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(modelShaderProgram->shaderProgram, "projectionMatrix"), 1,
		GL_FALSE, glm::value_ptr(projectionMatrix));

	glUniformMatrix4fv(glGetUniformLocation(modelShaderProgram->shaderProgram, "viewMatrix"), 1,
		GL_FALSE, glm::value_ptr(viewMatrix));

	mat4 modelMatrix = mat4(1.0f);
	modelMatrix = translate(modelMatrix, vec3(0.0f, -3.0f, 0.0f));
	modelMatrix = scale(modelMatrix, vec3(4.0f, 4.0f, 4.0f));

	glUniformMatrix4fv(glGetUniformLocation(modelShaderProgram->shaderProgram, "modelMatrix"), 1,
		GL_FALSE, glm::value_ptr(modelMatrix));

	planet->DrawModel(instancingShaderProgram);

	// Draw meteorites
	/*for (unsigned int i = 0; i < amount; i++)
	{
		// Instance Part 2
		glUniformMatrix4fv(glGetUniformLocation(instancingShaderProgram->shaderProgram, "modelMatrix"), 1,
			GL_FALSE, glm::value_ptr(modelMatrices[i]));

		rock->DrawModel(instancingShaderProgram);
	}*/

	glUseProgram(instancingShaderProgram->shaderProgram);

	glUniform1i(glGetUniformLocation(instancingShaderProgram->shaderProgram, "textureImage"), 0);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, rock->texturesLoaded[0].textureID);

	// Draw meteorites
	for (unsigned int i = 0; i < rock->meshes.size(); i++)
	{
		// Instance Part 3
		glBindVertexArray(rock->meshes[i].VAO);

		glDrawElementsInstanced(GL_TRIANGLES, rock->meshes[i].indices.size(), 
			GL_UNSIGNED_INT, 0, amount);

		glBindVertexArray(0);
	}

}
