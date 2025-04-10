#pragma once

#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <map>

#include "Texture2D.h"
#include "ShaderProgram.h"

/// Holds all state information relevant to a character as loaded using FreeType
struct CharacterStruct 
{
    unsigned int TextureID; // ID handle of the glyph texture
    ivec2 Size;      // size of glyph
    ivec2 Bearing;   // offset from baseline to left/top of glyph
    unsigned int Advance;   // horizontal offset to advance to next glyph
};

class TextRenderer
{
public:
    // holds a list of pre-compiled Characters
    map<char, CharacterStruct> Characters;

    // shader used for text rendering
    ShaderProgram TextShader;

    TextRenderer(unsigned int width, unsigned int height);

    // pre-compiles a list of characters from the given font
    void Load(string font, unsigned int fontSize);

    // renders a string of text using the precompiled list of characters
    void RenderText(string text, float x, float y, float scale, vec3 color = vec3(1.0f));

private:
    unsigned int VAO, VBO;
};

#endif