#pragma once

#include <map>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "stb_image.h"

#include "ShaderProgram.h"

// Store the generated data somewhere in the application and query it whenever we want to render a character
struct Character 
{
	unsigned int TextureID; // ID handle of the glyph texture
	ivec2 Size; // Size of glyph
	ivec2 Bearing; // Offset from baseline to left/top of glyph
	unsigned int Advance; // Offset to advance to next glyph
};


class TextRendering
{
public:
	~TextRendering();

	static TextRendering* Instance();

	void InitializeTextRendering();
	void ShowTextRendering();

private:
	TextRendering();

	void RenderText(ShaderProgram* shaderProgram, string text, float x, float y, float scale, vec3 color);

	static TextRendering* textRenderingInstance;

	ShaderProgram* textRenderingShader;

	FT_Library ft;
	FT_Face face;

	map<char, Character> Characters;

	unsigned int texture;

	mat4 projection;

	unsigned int VAO, VBO;
};

typedef TextRendering RenderText;