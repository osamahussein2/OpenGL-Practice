#include "AntiAliasing.h"
#include "Camera.h"

/* Some models have jagged saw-like patterns along the edges and the reason these jagged edges appear is due to how the 
rasterizer transforms the vertex data into actual fragments behind the scene. The effect, of clearly seeing the pixel 
formations an edge is composed of, is called aliasing. There are quite a few techniques out there called anti-aliasing 
techniques that fight this aliasing behavior by producing smoother edges */

/* We had a technique called super sample anti-aliasing (SSAA) that temporarily uses a much higher resolution render buffer 
to render the scene in (super sampling). Then when the full scene is rendered, the resolution is downsampled back to the 
normal resolution. This extra resolution was used to prevent these jagged edges. While it did provide us with a solution to 
the aliasing problem, it came with a major performance drawback since we have to draw a lot more fragments than usual. */

/* This technique did give birth to a more modern technique called multisample anti-aliasing or MSAA that borrows from the 
concepts behind SSAA while implementing a much more efficient approach */

/* The rasterizer is the combination of all algorithms and processes that sit between your final processed vertices and the 
fragment shader. The rasterizer takes all vertices belonging to a single primitive and transforms this to a set of fragments.

What multisampling does, is not use a single sampling point for determining coverage of the triangle, but multiple sample 
points */

AntiAliasing::AntiAliasing() : antiAliasingShaderProgram(new ShaderProgram()), 
antiAliasingPostShaderProgram(new ShaderProgram()), cubeVAO(NULL), cubeVBO(NULL)
{
    cubeVertices = { NULL };
}

AntiAliasing::~AntiAliasing()
{
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
}

void AntiAliasing::InitializeAntiAliasing()
{
	glEnable(GL_DEPTH_TEST);

	/* If we want to use MSAA (multi sampling anti aliasing) in OpenGL we need to use a buffer that is able to store more 
	than one sample value per pixel. We need a new type of buffer that can store a given amount of multisamples and this is 
	called a multisample buffer */

	/* Now that we asked GLFW for multisampled buffers we need to enable multisampling by calling glEnable with
	GL_MULTISAMPLE (Anti-Aliasing Part 1 only) */
	//glEnable(GL_MULTISAMPLE);

    antiAliasingShaderProgram->InitializeShaderProgram(new VertexShaderLoader("AntiAliasingVertexShader.glsl"), new
        FragmentShaderLoader("AntiAliasingFragmentShader.glsl"));

    antiAliasingPostShaderProgram->InitializeShaderProgram(new VertexShaderLoader("AntiAliasingPostVertexShader.glsl"), new
        FragmentShaderLoader("AntiAliasingPostFragmentShader.glsl"));

	cubeVertices =
	{
        // positions       
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
	};

    quadVertices = 
    {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates

        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    // Generate the quad VAO and VBO using the quad's vertices (Anti-Aliasing Part 2)
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Configure MSAA framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a multisampled color attachment texture
    glGenTextures(1, &textureColorBufferMultiSampled);

    /* To create a texture that supports storage of multiple sample points we use glTexImage2DMultisample instead of
    glTexImage2D that accepts GL_TEXTURE_2D_MULTISAMPLE as its texture target */
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);

    /* The second argument sets the number of samples we’d like the texture to have. If the last argument is set to GL_TRUE,
    the image will use identical sample locations and the same number of subsamples for each texel */
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, 1280, 960, GL_TRUE);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    /* To attach a multisampled texture to a framebuffer we use glFramebufferTexture2D, but this time with
    GL_TEXTURE_2D_MULTISAMPLE as the texture type */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, 
        textureColorBufferMultiSampled, 0);

    // Create a (also multisampled) renderbuffer object for depth and stencil attachments
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);

    /* It is even quite easy since all we need to change is glRenderbufferStorage to glRenderbufferStorageMultisample when
    we configure the (currently bound) renderbuffer’s memory storage (second parameter passes in how many samples we want
    to use) */
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, 1280, 960);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Configure second post-processing framebuffer
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

    // Create a color attachment texture
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 960, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // We only need a color buffer here
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << endl;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Use anti aliasing post shader program to set the uniform of the sampler2D in the fragment shader
    //glUseProgram(antiAliasingPostShaderProgram->shaderProgram);
    //glUniform1i(glGetUniformLocation(antiAliasingPostShaderProgram->shaderProgram, "screenTexture"), 0);

    // LEARNING PURPOSES ONLY BELOW

    /* To create a texture that supports storage of multiple sample points we use glTexImage2DMultisample instead of 
    glTexImage2D that accepts GL_TEXTURE_2D_MULTISAMPLE as its texture target */
    //glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);

    /* The second argument sets the number of samples we’d like the texture to have. If the last argument is set to GL_TRUE, 
    the image will use identical sample locations and the same number of subsamples for each texel */
    //glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
    //glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    /* To attach a multisampled texture to a framebuffer we use glFramebufferTexture2D, but this time with 
    GL_TEXTURE_2D_MULTISAMPLE as the texture type */
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);

    /* It is even quite easy since all we need to change is glRenderbufferStorage to glRenderbufferStorageMultisample when 
    we configure the (currently bound) renderbuffer’s memory storage (second parameter passes in how many samples we want 
    to use) */
    //glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);

    /* A multisampled image contains much more information than a normal image so what we need to do is downscale or resolve 
    the image. Resolving a multisampled framebuffer is generally done through glBlitFramebuffer that copies a region from one 
    framebuffer to the other while also resolving any multisampled buffers */

    /* glBlitFramebuffer transfers a given source region defined by 4 screen-space coordinates to a given target region also 
    defined by 4 screen-space coordinates. The glBlitFramebuffer function reads from those two targets to determine which is 
    the source and which is the target framebuffer. We could then transfer the multisampled framebuffer output to the actual 
    screen by blitting the image to the default framebuffer */
    /*glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampledFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);*/

}

void AntiAliasing::RenderAntiAliasing()
{
    // Anti-Aliasing Part 2
    
    // Draw scene as normal in multisampled buffers
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Set transformation matrices (Anti-Aliasing Part 1)
    glUseProgram(antiAliasingShaderProgram->shaderProgram);

    mat4 projectionMatrix = perspective(radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 1000.0f);
    mat4 viewMatrix = Camera::CameraLookAt();
    mat4 modelMatrix = mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(antiAliasingShaderProgram->shaderProgram, "projectionMatrix"), 1, GL_FALSE,
        value_ptr(projectionMatrix));

    glUniformMatrix4fv(glGetUniformLocation(antiAliasingShaderProgram->shaderProgram, "viewMatrix"), 1, GL_FALSE,
        value_ptr(viewMatrix));

    glUniformMatrix4fv(glGetUniformLocation(antiAliasingShaderProgram->shaderProgram, "modelMatrix"), 1, GL_FALSE, 
        value_ptr(modelMatrix));

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Anti-Aliasing Part 2
    /* A multisampled image contains much more information than a normal image so what we need to do is downscale or resolve 
    the image. Resolving a multisampled framebuffer is generally done through glBlitFramebuffer that copies a region from one
    framebuffer to the other while also resolving any multisampled buffers */
    
    /* glBlitFramebuffer transfers a given source region defined by 4 screen-space coordinates to a given target region also
    defined by 4 screen-space coordinates. The glBlitFramebuffer function reads from those two targets to determine which is
    the source and which is the target framebuffer. We could then transfer the multisampled framebuffer output to the actual
    screen by blitting the image to the default framebuffer */

    // 1280 is screen width and 960 is screen height I set the window to
    // Blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored in screenTexture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
    glBlitFramebuffer(0, 0, 1280, 960, 0, 0, 1280, 960, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Render quad with scene's visuals as its texture image
    // Since the scene is stored in 2D texture, we can use that for post-processing if necessary
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // Draw screen quad
    glUseProgram(antiAliasingPostShaderProgram->shaderProgram);

    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, screenTexture); // use the now resolved color attachment as the quad's texture
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
