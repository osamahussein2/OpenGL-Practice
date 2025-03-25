#pragma once

#include <glad/glad.h>
#include <glfw3.h>

#include "Texture2D.h"
#include "ShaderProgram.h"

class SpriteRenderer
{
public:
	SpriteRenderer(ShaderProgram shader_);
	~SpriteRenderer();

	void DrawSprite(Texture2D texture, vec2 position, vec2 size = vec2(10.0f, 10.0f), float rotate_ = 0.0f, vec3 color = vec3(1.0f));
private:
	ShaderProgram shader;
	unsigned int quadVAO;

	void InitializeRenderData();
};
