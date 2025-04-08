#include "Postprocessing.h"

/* Rendering steps are in order from 1-6:

	1. Bind to multisampled framebuffer
	2. Render game as normal
	3. Blit multisampled framebuffer to normal framebuffer with texture attachment
	4. Unbind framebuffer (use default framebuffer)
	5. Use color buffer texture from normal framebuffer in postprocessing shader
	6. Render quad of screen-size as output of postprocessing shader

 */

/* The postprocessing shader allows for three type of effects: shake, confuse, and chaos

shake: slightly shakes the scene with a small blur

confuse: inverses the colors of the scene, but also the x and y axis

chaos: makes use of an edge detection kernel to create interesting visuals and also moves the textured image in a
circular fashion for an interesting chaotic effect */

Postprocessing::Postprocessing(ShaderProgram shader, unsigned int width, unsigned int height) : PostProcessingShader(shader), Texture(), Width(width), Height(height),
Confuse(false), Chaos(false), Shake(false)
{
    // initialize renderbuffer/framebuffer object
    glGenFramebuffers(1, &this->MSFBO);
    glGenFramebuffers(1, &this->FBO);
    glGenRenderbuffers(1, &this->RBO);

    // initialize renderbuffer storage with a multisampled color buffer (don't need a depth/stencil buffer)
    glBindFramebuffer(GL_FRAMEBUFFER, this->MSFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);

    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB, width, height); // allocate storage for render buffer object
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, this->RBO); // attach MS render buffer object to framebuffer
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::POSTPROCESSOR: Failed to initialize MSFBO" << std::endl;

    // Also initialize the FBO/texture to blit multisampled color-buffer to; used for shader operations (for postprocessing effects)
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);

    this->Texture.Generate(width, height, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->Texture.textureID, 0); // attach texture to framebuffer as its color attachment
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::POSTPROCESSOR: Failed to initialize FBO" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // initialize render data and uniforms
    this->InitializeRenderData();

    glUseProgram(PostProcessingShader.shaderProgram);
    glUniform1i(glGetUniformLocation(PostProcessingShader.shaderProgram, "scene"), 0);

    float offset = 1.0f / 300.0f;

    float offsets[9][2] =
    {
        { -offset,  offset  },  // top-left
        {  0.0f,    offset  },  // top-center
        {  offset,  offset  },  // top-right
        { -offset,  0.0f    },  // center-left
        {  0.0f,    0.0f    },  // center-center
        {  offset,  0.0f    },  // center - right
        { -offset, -offset  },  // bottom-left
        {  0.0f,   -offset  },  // bottom-center
        {  offset, -offset  }   // bottom-right    
    };

    glUniform2fv(glGetUniformLocation(this->PostProcessingShader.shaderProgram, "offsets"), 9, (float*)&offsets);

    int edgeKernel[9] =
    {
        -1, -1, -1,
        -1,  8, -1,
        -1, -1, -1
    };

    glUniform1iv(glGetUniformLocation(this->PostProcessingShader.shaderProgram, "edge_kernel"), 9, edgeKernel);

    float blurKernel[9] =
    {
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
        2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
    };

    glUniform1fv(glGetUniformLocation(this->PostProcessingShader.shaderProgram, "blur_kernel"), 9, blurKernel);
}

void Postprocessing::BeginRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->MSFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Postprocessing::EndRender()
{
    // now resolve multisampled color-buffer into intermediate FBO to store to texture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->MSFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->FBO);
    glBlitFramebuffer(0, 0, this->Width, this->Height, 0, 0, this->Width, this->Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // binds both READ and WRITE framebuffer to default framebuffer
}

void Postprocessing::RenderPostprocessing(float time)
{
    // set uniforms/options
    glUseProgram(this->PostProcessingShader.shaderProgram);
    glUniform1f(glGetUniformLocation(this->PostProcessingShader.shaderProgram, "time"), time);
    glUniform1i(glGetUniformLocation(this->PostProcessingShader.shaderProgram, "confuse"), this->Confuse);
    glUniform1i(glGetUniformLocation(this->PostProcessingShader.shaderProgram, "chaos"), this->Chaos);
    glUniform1i(glGetUniformLocation(this->PostProcessingShader.shaderProgram, "shake"), this->Shake);

    // render textured quad
    glActiveTexture(GL_TEXTURE0);
    this->Texture.Bind();

    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Postprocessing::InitializeRenderData()
{
    // configure VAO/VBO
    unsigned int VBO;

    array<float, 24> vertices = 
    {
        // pos        // tex
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,

        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    glBindVertexArray(this->VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}