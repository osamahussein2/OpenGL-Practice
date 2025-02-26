#include "HDR.h"
#include "Camera.h"

/* Monitors (non-HDR) are limited to display colors in the range of 0.0 and 1.0, but there is no such limitation in lighting 
equations. By allowing fragment colors to exceed 1.0 we have a much higher range of color values available to work in known 
as high dynamic range (HDR). With high dynamic range, bright things can be really bright, dark things can be really dark, 
and details can be seen in both */

/* High dynamic range rendering works a bit like that. We allow for a much larger range of color
values to render to, collecting a large range of dark and bright details of a scene, and at the end we
transform all the HDR values back to the low dynamic range (LDR) of [0.0, 1.0] */

/* This process of converting HDR values to LDR values is called tone mapping and a large collection of tone
mapping algorithms exist that aim to preserve most HDR details during the conversion process.
These tone mapping algorithms often involve an exposure parameter that selectively favors dark or
bright regions */

HDR* HDR::hdrInstance = NULL;

HDR::HDR() : hdr(true), exposure(1.0f), shaders{ new ShaderProgram(), new ShaderProgram() }, cubeVAO(0), cubeVBO(0)
{
}

HDR::~HDR()
{
}

HDR* HDR::Instance()
{
	if (hdrInstance == NULL)
	{
		hdrInstance = new HDR();
	}

	return hdrInstance;
}

void HDR::InitializeHDR()
{
	glEnable(GL_DEPTH_TEST);

	// Load the HDR lighting shaders here
	shaders[0]->InitializeShaderProgram(new VertexShaderLoader("HDRLightingVertexShader.glsl"),
		new FragmentShaderLoader("HDRLightingFragmentShader.glsl"));

	// Load the HDR shaders here
	shaders[1]->InitializeShaderProgram(new VertexShaderLoader("HDRVertexShader.glsl"),
		new FragmentShaderLoader("HDRFragmentShader.glsl"));

	woodTexture = LoadTexture("Textures/Wood.png", true);

    glGenFramebuffers(1, &hdrFBO);

    // Create floating point color buffer
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);

    /* With a floating point framebuffer with 32 bits per color component (when using GL_RGB32F or GL_RGBA32F) we’re using 4 
    times more memory for storing color values */

    /* When the internal format of a framebuffer’s color buffer is specified as GL_RGB16F, GL_RGBA16F, GL_RGB32F, or 
    GL_RGBA32F the framebuffer is known as a floating point framebuffer that can store floating point values outside the 
    default range of 0.0 and 1.0 */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 960, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create depth buffer (renderbuffer)
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 960);

    // Attach buffers
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
        cout << "Framebuffer not complete!" << endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // positions
    lightPositions.push_back(glm::vec3(0.0f, 0.0f, 49.5f)); // back light
    lightPositions.push_back(glm::vec3(-1.4f, -1.9f, 9.0f));
    lightPositions.push_back(glm::vec3(0.0f, -1.8f, 4.0f));
    lightPositions.push_back(glm::vec3(0.8f, -1.7f, 6.0f));

    // colors
    lightColors.push_back(glm::vec3(200.0f, 200.0f, 200.0f));
    lightColors.push_back(glm::vec3(0.1f, 0.0f, 0.0f));
    lightColors.push_back(glm::vec3(0.0f, 0.0f, 0.2f));
    lightColors.push_back(glm::vec3(0.0f, 0.1f, 0.0f));

    glUseProgram(shaders[0]->shaderProgram);
    glUniform1i(glGetUniformLocation(shaders[0]->shaderProgram, "diffuseTexture"), 0);

    glUseProgram(shaders[1]->shaderProgram);
    glUniform1i(glGetUniformLocation(shaders[1]->shaderProgram, "hdrBuffer"), 0);
}

void HDR::RenderHDR()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render scene into floating point framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 projection = glm::perspective(glm::radians(Camera::fieldOfView), GLfloat(1280 / 960), 0.1f, 100.0f);
    glm::mat4 view = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);

    glUseProgram(shaders[0]->shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaders[0]->shaderProgram, "projection"), 1, GL_FALSE, value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaders[0]->shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, woodTexture);

    // set lighting uniforms
    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
        glUniform3fv(glGetUniformLocation(shaders[0]->shaderProgram, 
            ("lights[" + to_string(i) + "].Position").c_str()), 1, value_ptr(lightPositions[i]));

        glUniform3fv(glGetUniformLocation(shaders[0]->shaderProgram, 
            ("lights[" + to_string(i) + "].Color").c_str()), 1, value_ptr(lightColors[i]));
    }

    glUniform3fv(glGetUniformLocation(shaders[0]->shaderProgram, "viewPos"), 1, value_ptr(Camera::cameraPosition));

    // render tunnel
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 25.0));
    model = glm::scale(model, glm::vec3(2.5f, 2.5f, 27.5f));

    glUniformMatrix4fv(glGetUniformLocation(shaders[0]->shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
    glUniform1i(glGetUniformLocation(shaders[0]->shaderProgram, "inverse_normals"), true);

    RenderCube();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaders[1]->shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glUniform1i(glGetUniformLocation(shaders[1]->shaderProgram, "hdr"), hdr);
    glUniform1f(glGetUniformLocation(shaders[1]->shaderProgram, "exposure"), exposure);
    RenderQuad();

    std::cout << "hdr: " << (hdr ? "on" : "off") << "| exposure: " << exposure << std::endl;
}

unsigned int HDR::LoadTexture(const char* path_, bool gammaCorrection)
{
	unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path_, &width, &height, &nrComponents, 0);
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
        std::cout << "Texture failed to load at path: " << path_ << std::endl;
        stbi_image_free(data);
    }

	return textureID;
}

void HDR::RenderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
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
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
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

    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void HDR::RenderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
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
