#include "Bloom.h"
#include "Camera.h"

Bloom* Bloom::bloomInstance = NULL;

/* One way to distinguish bright light sources on a monitor is by making them glow; the light then bleeds around the light 
source. This effectively gives the viewer the illusion these light sources or bright regions are intensely bright. This light 
bleeding, or glow effect, is achieved with a post-processing effect called Bloom. Bloom gives all brightly lit regions of a 
scene a glow-like effect. Bloom gives noticeable visual cues about the brightness of objects */

/* Multiple Render Targets (MRT) that allows us to specify more than one fragment shader output; this gives an option of
extracting the first two images in a single render pass */

Bloom::Bloom() : bloom(true), exposure(1.0f), cubeVAO(0), cubeVBO(0), quadVAO(0), quadVBO(0), 
bloomShaders{ new ShaderProgram(), new ShaderProgram(), new ShaderProgram(), new ShaderProgram() }
{
}

Bloom::~Bloom()
{

}

Bloom* Bloom::Instance()
{
	if (bloomInstance == NULL)
	{
		bloomInstance = new Bloom();
		return bloomInstance;
	}

	return bloomInstance;
}

void Bloom::InitializeBloom()
{
	glEnable(GL_DEPTH_TEST);

	// Bloom shader
	bloomShaders[0]->InitializeShaderProgram(new VertexShaderLoader("BloomVertexShader.glsl"), 
		new FragmentShaderLoader("BloomFragmentShader.glsl"));

	// Bloom shader using a light box
	bloomShaders[1]->InitializeShaderProgram(new VertexShaderLoader("BloomVertexShader.glsl"),
		new FragmentShaderLoader("BloomLightboxFragmentShader.glsl"));

	// Blur shader
	bloomShaders[2]->InitializeShaderProgram(new VertexShaderLoader("BloomBlurVertexShader.glsl"),
		new FragmentShaderLoader("BloomBlurFragmentShader.glsl"));

	// Bloom final shader
	bloomShaders[3]->InitializeShaderProgram(new VertexShaderLoader("BloomFinalVertexShader.glsl"),
		new FragmentShaderLoader("BloomFinalFragmentShader.glsl"));

	woodTexture = LoadTexture("Textures/Wood.png", true);
	containerTexture = LoadTexture("Textures/container2.png", true);

    // Initialize frame buffer (floating point)
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // Create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    glGenTextures(2, colorBuffers);

    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 960, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Clamp to the edge as the blur filter would otherwise sample repeated texture values
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    // Create and attach depth buffer (renderbuffer)
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 960);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    attachments[0] = GL_COLOR_ATTACHMENT0;
    attachments[1] = GL_COLOR_ATTACHMENT1;

    glDrawBuffers(2, attachments);

    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "Framebuffer not complete!" << endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);

    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 960, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);

        // Check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    // positions
    lightPositions.push_back(vec3(0.0f, 0.5f, 1.5f));
    lightPositions.push_back(vec3(-4.0f, 0.5f, -3.0f));
    lightPositions.push_back(vec3(3.0f, 0.5f, 1.0f));
    lightPositions.push_back(vec3(-0.8f, 2.4f, -1.0f));

    // colors
    lightColors.push_back(vec3(5.0f, 5.0f, 5.0f));
    lightColors.push_back(vec3(10.0f, 0.0f, 0.0f));
    lightColors.push_back(vec3(0.0f, 0.0f, 15.0f));
    lightColors.push_back(vec3(0.0f, 5.0f, 0.0f));

    glUseProgram(bloomShaders[0]->shaderProgram);
    glUniform1i(glGetUniformLocation(bloomShaders[0]->shaderProgram, "diffuseTexture"), 0);

    glUseProgram(bloomShaders[2]->shaderProgram);
    glUniform1i(glGetUniformLocation(bloomShaders[2]->shaderProgram, "image"), 0);

    glUseProgram(bloomShaders[3]->shaderProgram);
    glUniform1i(glGetUniformLocation(bloomShaders[3]->shaderProgram, "scene"), 0);
    glUniform1i(glGetUniformLocation(bloomShaders[3]->shaderProgram, "bloomBlur"), 1);
}

void Bloom::RenderBloom()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render scene into floating point framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 100.0f);
    glm::mat4 view = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);
    glm::mat4 model = glm::mat4(1.0f);
    
    glUseProgram(bloomShaders[0]->shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, "projection"), 1, GL_FALSE, value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, woodTexture);

    // Set lighting uniforms
    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
        glUniform3fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, 
            ("lights[" + std::to_string(i) + "].position").c_str()), 1, value_ptr(lightPositions[i]));

        glUniform3fv(glGetUniformLocation(bloomShaders[0]->shaderProgram,
            ("lights[" + std::to_string(i) + "].color").c_str()), 1, value_ptr(lightColors[i]));
    }

    glUniform3fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, "viewPos"), 1, value_ptr(Camera::cameraPosition));
    
    // Create one large cube that acts as the floor
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
    model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));

    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
    RenderCube();

    // then create multiple cubes as the scenery
    glBindTexture(GL_TEXTURE_2D, containerTexture);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));

    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
    RenderCube();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.5f));
    
    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
    RenderCube();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    
    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
    RenderCube();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
    model = glm::rotate(model, glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(1.25));
    
    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
    RenderCube();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
    model = glm::rotate(model, glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    
    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
    RenderCube();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    
    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[0]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
    RenderCube();

    // Finally show all the light sources as bright cubes
    glUseProgram(bloomShaders[1]->shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[1]->shaderProgram, "projection"), 1, GL_FALSE, value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(bloomShaders[1]->shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(lightPositions[i]));
        model = glm::scale(model, glm::vec3(0.25f));
        
        glUniformMatrix4fv(glGetUniformLocation(bloomShaders[1]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
        glUniform3fv(glGetUniformLocation(bloomShaders[1]->shaderProgram, "lightColor"), 1, value_ptr(lightColors[i]));
        RenderCube();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Blur bright fragments with two-pass Gaussian Blur
    bool horizontal = true, first_iteration = true;
    unsigned int amount = 10;

    glUseProgram(bloomShaders[2]->shaderProgram);

    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        glUniform1i(glGetUniformLocation(bloomShaders[2]->shaderProgram, "horizontal"), horizontal);

        // Bind texture of other framebuffer (or scene if first iteration)
        glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
        
        RenderQuad();

        horizontal = !horizontal;
        if (first_iteration) first_iteration = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(bloomShaders[3]->shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

    glUniform1i(glGetUniformLocation(bloomShaders[3]->shaderProgram, "bloom"), bloom);
    glUniform1f(glGetUniformLocation(bloomShaders[3]->shaderProgram, "exposure"), exposure);
    RenderQuad();

    cout << "bloom: " << (bloom ? "on" : "off") << "| exposure: " << exposure << endl;
}

void Bloom::RenderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        array<float, 288> vertices = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
             // bottom face
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
              1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             // top face
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
              1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
              1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
              1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Bloom::RenderQuad()
{
    if (quadVAO == 0)
    {
        array<float, 20> quadVertices = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int Bloom::LoadTexture(const char* path, bool gammaCorrection)
{
	unsigned int textureID;

    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

	return textureID;
}