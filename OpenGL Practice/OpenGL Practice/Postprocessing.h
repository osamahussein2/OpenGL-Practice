#pragma once

#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include <glad/glad.h>
#include <glm.hpp>

#include "Texture2D.h"
#include "SpriteRenderer.h"
#include "ShaderProgram.h"

class Postprocessing
{
public:
    ShaderProgram PostProcessingShader;
    Texture2D Texture;
    unsigned int Width, Height;

    // Postprocessing options
    bool Confuse, Chaos, Shake;

    Postprocessing(ShaderProgram shader, unsigned int width, unsigned int height);

    // Prepares the post processing framebuffer operations before rendering the game
    void BeginRender();

    // Should be called after rendering the game, so it stores all the rendered data into a texture object
    void EndRender();

    // Renders the post processing texture quad (as a screen-encompassing large sprite)
    void RenderPostprocessing(float time);

private:
    unsigned int MSFBO, FBO; // MSFBO = Multisampled FBO. FBO is regular, used for blitting MS color-buffer to texture
    unsigned int RBO; // RBO is used for multisampled color buffer
    unsigned int VAO;

    // Initialize quad for rendering postprocessing texture
    void InitializeRenderData();
};

#endif