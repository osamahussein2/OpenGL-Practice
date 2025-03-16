#include "TextRendering.h"
#include "Camera.h"

/* A bitmap font contains all character symbols we want to use in predefined regions of the texture. These character symbols 
of the font are known as glyphs. Each glyph has a specific region of texture coordinates associated with them */

/* FreeType is a software development library that is able to load fonts, render them to bitmaps, and provide support for 
several font-related operations. It is a popular library used by Mac OS X, Java, PlayStation, Linux, and Android to name a 
few. What makes FreeType particularly attractive is that it is able to load TrueType fonts. */

TextRendering* TextRendering::textRenderingInstance = NULL;

TextRendering::TextRendering() : textRenderingShader(new ShaderProgram()), ft(NULL), face(NULL), texture(0), VAO(0), VBO(0),
projection(mat4(0.0f))
{
}

TextRendering::~TextRendering()
{
	delete textRenderingShader;

	ft = NULL;
	face = NULL;

	glDeleteTextures(1, &texture);

	projection = mat4(NULL);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

TextRendering* TextRendering::Instance()
{
	if (textRenderingInstance == NULL)
	{
		textRenderingInstance = new TextRendering();
		return textRenderingInstance;
	}

	return textRenderingInstance;
}

void TextRendering::InitializeTextRendering()
{
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize the text rendering shader
	textRenderingShader->InitializeShaderProgram(new VertexShaderLoader("TextRenderingVertexShader.glsl"),
		new FragmentShaderLoader("TextRenderingFragmentShader.glsl"));

	/* Use an orthographic projection matrix for rendering text since we (usually) do not need perspective, and using an
	orthographic projection matrix also allows us to specify all vertex coordinates in screen coordinates */

	/* Set the projection matrix’s bottom parameter to 0.0f and its top parameter equal to the window’s height. The result is
	that we specify coordinates with y values ranging from the bottom part of the screen (0.0f) to the top part of the screen.
	This means that the point (0.0, 0.0) now corresponds to the bottom-left corner */
	projection = ortho(0.0f, 800.0f, 0.0f, 600.0f);

	glUseProgram(textRenderingShader->shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(textRenderingShader->shaderProgram, "projection"), 1, GL_FALSE, 
		value_ptr(projection));

	/* To load a font, all we have to do is initialize the FreeType library and load the font as a face as FreeType likes to 
	call it */

	if (FT_Init_FreeType(&ft)) cout << "ERROR::FREETYPE: Could not init FreeType Library" << endl;
	if (FT_New_Face(ft, "Fonts/Antonio-Bold.ttf", 0, &face)) cout << "ERROR::FREETYPE: Failed to load font" << endl;

	// Once we’ve loaded the face, we should define the pixel font size we’d like to extract from this face

	/* The function sets the font’s width and height parameters. Setting the width to 0 lets the face dynamically calculate 
	the width based on the given height */
	FT_Set_Pixel_Sizes(face, 0, 48);

	/* A FreeType face hosts a collection of glyphs.We can set one of those glyphs as the active glyph by calling
	FT_Load_Char. By setting FT_LOAD_RENDER as one of the loading flags, we tell FreeType to create an 8-bit grayscale 
	bitmap image for us that we can access via face->glyph->bitmap (LEARNING PURPOSES ONLY) */
	//if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) cout << "ERROR::FREETYTPE: Failed to load Glyph" << endl;

	// If we use a single byte to represent the colors of a texture we do need to take care of a restriction of OpenGL
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // no byte-alignment restriction

	/* For each character, we generate a texture and store its relevant data into a Character struct that we add to the
	Characters map. This way, all data required to render each character is stored for later use */
	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}

		// generate texture
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		/* The bitmap generated from the glyph is a grayscale 8-bit image where each color is represented by a single byte. 
		For this reason we’d like to store each byte of the bitmap buffer as the texture’s single color value. We accomplish 
		this by creating a texture where each byte corresponds to the texture color’s red component (first byte of its color 
		vector) */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, 
			GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// now store character for later use
		Character character = 
		{
			texture, ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), 
			ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), face->glyph->advance.x
		};

		Characters.insert(std::pair<char, Character>(c, character));
	}

	// Clear FreeType’s resources once the glyphs has finished processing
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// Configure VAO and VBO for texture quads
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	/* The 2D quad requires 6 vertices of 4 floats each, so we reserve 6 * 4 floats of memory. Because we’ll be updating the 
	content of the VBO’s memory quite often we’ll allocate the memory with GL_DYNAMIC_DRAW */
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void TextRendering::ShowTextRendering()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	RenderText(textRenderingShader, "This is sample text", 25.0f, 25.0f, 1.0f, vec3(0.5, 0.8f, 0.2f));
	RenderText(textRenderingShader, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, vec3(0.3, 0.7f, 0.9f));
}

/* To render a character, we extract the corresponding Character struct of the Characters map and calculate the quad’s 
dimensions using the character’s metrics. With the quad’s calculated dimensions we dynamically generate a set of 6 vertices 
that we use to update the content of the memory managed by the VBO using glBufferSubData */
void TextRendering::RenderText(ShaderProgram* shaderProgram, string text, float x, float y, float scale, vec3 color)
{
	// activate corresponding render state
	glUseProgram(shaderProgram->shaderProgram);

	glUniform3f(glGetUniformLocation(shaderProgram->shaderProgram, "textColor"), color.x, color.y, color.z);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// iterate through all characters
	string::const_iterator c;

	for (c = text.begin(); c != text.end(); c++)
	{
		// Calculate the origin position of the quad (as xpos and ypos) and the quad’s size (as w and h)
		Character ch = Characters[*c];
		float xpos = x + ch.Bearing.x * scale;

		/* Some characters (like ’p’ or ’g’) are rendered slightly below the baseline, so the quad should also be positioned 
		slightly below RenderText’s y value */
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
		
		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		/* Generate a set of 6 vertices to form the 2D quad; note that we scale each metric by scale. We then update the 
		content of the VBO and render the quad */

		// update VBO for each character
		float vertices[6][4] = 
		{
			{ xpos, ypos + h, 0.0f, 0.0f }, { xpos, ypos, 0.0f, 1.0f }, { xpos + w, ypos, 1.0f, 1.0f },
			{ xpos, ypos + h, 0.0f, 0.0f }, { xpos + w, ypos, 1.0f, 1.0f }, { xpos + w, ypos + h, 1.0f, 0.0f }
		};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// advance cursors for next glyph (advance is 1/64 pixels)

		x += (ch.Advance >> 6) * scale; // bitshift by 6 (2^6 = 64)
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/* The text rendering technique with TrueType fonts using the FreeType library is flexible, scalable, and works with many 
character encodings. However, this approach is likely going to be overkill for your application as we generate and render 
textures for each glyph. Performance-wise, bitmap fonts are preferable as we only need one texture for all our glyphs. The 
best approach would be to combine the two approaches by dynamically generating a bitmap font texture featuring all character 
glyphs as loaded with FreeType. This saves the renderer from a significant amount of texture switches and, based on how tight 
each glyph is packed, could save quite some performance. */

/* Another issue with FreeType font bitmaps is that the glyph textures are stored with a fixed font size, so a significant 
amount of scaling may introduce jagged edges. Furthermore, rotations applied to the glyphs will cause them to appear blurry.
This can be mitigated by, instead of storing the actual rasterized pixel colors, storing the distance to the closest glyph 
outline per pixel. This technique is called signed distance field fonts */