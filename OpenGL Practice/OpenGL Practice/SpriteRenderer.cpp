#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer(ShaderProgram shader_)
{
	shader = shader_;

	InitializeRenderData();
}

SpriteRenderer::~SpriteRenderer()
{
	glDeleteVertexArrays(1, &quadVAO);
}

void SpriteRenderer::InitializeRenderData()
{
	unsigned int VBO;

	array<float, 24> vertices = 
	{
		// position // texture coordinates
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	// Send the vertices to the GPU and configure the vertex attributes
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	glBindVertexArray(quadVAO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void SpriteRenderer::DrawSprite(Texture2D texture, vec2 position, vec2 size, float rotate_, vec3 color)
{
	glUseProgram(shader.shaderProgram);

	/* When trying to position objects somewhere in a scene with rotation and scaling transformations, it is advised to first scale, then rotate, 
	and finally translate the object. Because multiplying matrices occurs from right to left, we transform the matrix in reverse order: translate, 
	rotate, and then scale */
	mat4 modelMatrix = mat4(1.0f);
	modelMatrix = translate(modelMatrix, vec3(position, 0.0f));

	/* The origin of rotation is at the top-left of the quad, which produces undesirable results. What we want to do is move the origin of rotation to 
	the center of the quad so the quad neatly rotates around this origin, instead of rotating around the top-left of the quad. We solve this by translating 
	the quad by half its size first, so its center is at coordinate (0,0) before rotating */
	modelMatrix = translate(modelMatrix, vec3(0.5 * size.x, 0.5 * size.y, 0.0));

	modelMatrix = rotate(modelMatrix, radians(rotate_), vec3(0.0, 0.0, 1.0));
	modelMatrix = translate(modelMatrix, glm::vec3(-0.5 * size.x, -0.5 * size.y, 0.0));

	modelMatrix = scale(modelMatrix, glm::vec3(size, 1.0f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "modelMatrix"), 1, GL_FALSE, value_ptr(modelMatrix));
	glUniform3f(glGetUniformLocation(shader.shaderProgram, "spriteColor"), color.x, color.y, color.z);

	glActiveTexture(GL_TEXTURE0);
	texture.Bind();

	// Draw quad
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}