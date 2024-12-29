#include "Mesh.h"

Mesh::Mesh(vector<Vertex> vertices_, vector<unsigned int> indices_, vector<Texture> textures_) : 
	vertices(vertices_), indices(indices_), textures(textures_)
{
	// I typically like to initialize the variables of the constructors to NULL, 0, nullptr or false
	VAO = NULL;
	VBO = NULL;
	EBO = NULL;

	diffuseNumber = NULL;
	specularNumber = NULL;

	SetupMesh();
}

void Mesh::SetupMesh()
{
	glGenVertexArrays(1, &VAO);

	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	/* The offsetof macro passes in a struct name as the first argument, and the variable name inside the struct for 
	the second argument. */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0); // Position attribute location occurs at 0

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, meshNormal));
	glEnableVertexAttribArray(1); // Position attribute location occurs at 1

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, meshTextureCoordinates));
	glEnableVertexAttribArray(2); // Position attribute location occurs at 2

	glBindVertexArray(0);
}

void Mesh::DrawMesh(ShaderProgram *shaderProgram_)
{
	diffuseNumber = 1;
	specularNumber = 1;

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		// Active as many textures as we need as this loop iterates through the textures list
		glActiveTexture(GL_TEXTURE0 + i);

		string number;
		string name = textures[i].textureType;

		/* Basically, if the name of the string can find TextureDiffuse in my fragment shader, increment the number to
		loop through all the diffuse textures we set in the fragment shader. */
		if (name == "TextureDiffuse")
		{
			number = to_string(diffuseNumber++);
		}

		else if (name == "TextureSpecular")
		{
			number = to_string(specularNumber++);
		}

		glUniform1f(glGetUniformLocation(shaderProgram_->shaderProgram, (name + number).c_str()), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].textureID);
	}

	// Draw a mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}