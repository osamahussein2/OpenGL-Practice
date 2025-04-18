#include "TextRenderer.h"
#include <iostream>

#include <gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "ResourceManager.h"


TextRenderer::TextRenderer(unsigned int width, unsigned int height)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // load and configure shader
    this->TextShader = ResourceManager::LoadShader("Text2DVertexShader.glsl", "Text2DFragmentShader.glsl", nullptr, "text");

    glUseProgram(this->TextShader.shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(this->TextShader.shaderProgram, "projection"), 1, GL_FALSE,
        value_ptr(ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f)));

    glUniform1i(glGetUniformLocation(this->TextShader.shaderProgram, "text"), 0);

    // configure VAO/VBO for texture quads
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderer::Load(string font, unsigned int fontSize)
{
    // first clear the previously loaded Characters
    this->Characters.clear();

    // then initialize and load the FreeType library
    FT_Library ft;

    if (FT_Init_FreeType(&ft)) // all functions return a value different than 0 whenever an error occurred
        cout << "ERROR::FREETYPE: Could not init FreeType Library" << endl;

    // load font as face
    FT_Face face;

    if (FT_New_Face(ft, font.c_str(), 0, &face))
        cout << "ERROR::FREETYPE: Failed to load font" << endl;

    // set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // then for the first 128 ASCII characters, pre-load/compile their characters and store them
    for (GLubyte c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            cout << "ERROR::FREETYTPE: Failed to load Glyph" << endl;
            continue;
        }

        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0,
            GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // now store character for later use
        CharacterStruct character =
        {
            texture, ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), 
            ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), face->glyph->advance.x
        };

        Characters.insert(pair<char, CharacterStruct>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextRenderer::RenderText(string text, float x, float y, float scale, vec3 color)
{
    // activate corresponding render state	
    glUseProgram(this->TextShader.shaderProgram);
    glUniform3fv(glGetUniformLocation(TextShader.shaderProgram, "textColor"), 1, value_ptr(color));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(this->VAO);

    // iterate through all characters
    string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        CharacterStruct ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;

        /* A different orthographic projection matrix was used from the one in the text rendering chapter. In the 
        text rendering chapter all y values ranged from bottom to top, while in the Breakout game all y values range 
        from top to bottom with a y coordinate of 0.0 corresponding to the top edge of the screen. This means we have
        to slightly modify how we calculate the vertical offset */
        float ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            { xpos,     ypos,       0.0f, 0.0f },

            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f }
        };

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

        // Use glBufferSubData instead of glBufferData
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // now advance cursors for next glyph
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
