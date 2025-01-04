#include "GammaCorrection.h"
#include "Camera.h"

/* Doubling the input voltage resulted in a brightness equal to an exponential relationship of roughly 2.2 known as the 
gamma of a monitor */

/* The idea of gamma correction is to apply the inverse of the monitor’s gamma to the final output color before displaying 
to the monitor. We multiply each of the linear output colors by this inverse gamma curve (making them brighter) and as soon 
as the colors are displayed on the monitor, the monitor’s gamma curve is applied and the resulting colors become linear. We 
effectively brighten the intermediate colors so that as soon as the monitor darkens them, it balances all out */

/* A gamma value of 2.2 is a default gamma value that roughly estimates the average gamma of most displays. The color space 
as a result of this gamma of 2.2 is called the sRGB color space (not 100% exact, but it's close to a 100%) */

/* There are two ways to apply gamma correction in OpenGL:

	1. By using OpenGL’s built-in sRGB framebuffer support
	2. By doing the gamma correction inside the fragment shader(s)

*/

GammaCorrection* GammaCorrection::gammaCorrectionInstance = 0;

GammaCorrection::GammaCorrection() : gammaCorrectionShaderProgram(new ShaderProgram())
{

}

GammaCorrection::~GammaCorrection()
{
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
}

GammaCorrection* GammaCorrection::Instance()
{
	if (gammaCorrectionInstance == 0)
	{
		gammaCorrectionInstance = new GammaCorrection();
		return gammaCorrectionInstance;
	}

	return gammaCorrectionInstance;
}

void GammaCorrection::InitializeGamma()
{
	/* By enabling GL_FRAMEBUFFER_SRGB we tell OpenGL that each subsequent drawing command should first gamma correct 
	colors (from the sRGB color space) before storing them in color buffer(s). The sRGB is a color space that roughly 
	corresponds to a gamma of 2.2. After enabling GL_FRAMEBUFFER_SRGB, OpenGL automatically performs gamma correction after
	each fragment shader run to all subsequent framebuffers, including the default framebuffer (LEARNING PURPOSES ONLY)
	glEnable(GL_FRAMEBUFFER_SRGB);

	/* If we create a texture in OpenGL with any of the two sRGB texture formats (GL_SRGB and GL_SRGB_ALPHA), OpenGL will 
	automatically correct the colors to linear-space as soon as we use them, allowing us to properly work in linear space. 
	We can specify a texture as an sRGB texture like so: (LEARNING PURPOSES ONLY)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); */

	gammaCorrectionShaderProgram->InitializeShaderProgram(new VertexShaderLoader("GammaCorrectionVertexShader.glsl"),
		new FragmentShaderLoader("GammaCorrectionFragmentShader.glsl"));

    // Set up vertex data (and buffer(s)) and configure vertex attributes
    planeVertices = 
    {
        // Position (vec3)      // Normal (vec3)   // Texture Coordinate (vec2)
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
         10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
    };

    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(POSITION_ATTRIBUTE);
    glVertexAttribPointer(POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(NORMAL_ATTRIBUTE);
    glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(TEXTURE_COORDINATE_ATTRIBUTE);
    glVertexAttribPointer(TEXTURE_COORDINATE_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
        (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

}

void GammaCorrection::InitializeTextures(char const* path_, bool gammaCorrection_)
{
    glGenTextures(1, &floorTexture);

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
            internalFormat = gammaCorrection_ ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }

        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection_ ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, floorTexture);
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
}

void GammaCorrection::InitializeGammaCorrectedTextures(char const* path_, bool gammaCorrection_)
{
    glGenTextures(1, &floorTextureGammaCorrected);

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
            internalFormat = gammaCorrection_ ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }

        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection_ ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, floorTextureGammaCorrected);
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

    // Use the shader program after initializing the 2 textures and set the sampler2D uniform
    glUseProgram(gammaCorrectionShaderProgram->shaderProgram);
    glUniform1i(glGetUniformLocation(gammaCorrectionShaderProgram->shaderProgram, "floorTexture"), 0);
}

void GammaCorrection::InitializeLighting()
{
    lightPositions = 
    {
        vec3(-3.0f, 0.0f, 0.0f),
        vec3(-1.0f, 0.0f, 0.0f),
        vec3(1.0f, 0.0f, 0.0f),
        vec3(3.0f, 0.0f, 0.0f)
    };

    lightColors = 
    {
        vec3(0.25),
        vec3(0.5),
        vec3(0.75),
        vec3(1.0)
    };
}

void GammaCorrection::UseGammaShaderProgram()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gammaCorrectionShaderProgram->shaderProgram);

    mat4 projectionMatrix = perspective(radians(Camera::fieldOfView), float(1280/960), 0.1f, 100.0f);
    mat4 viewMatrix = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);

    glUniformMatrix4fv(glGetUniformLocation(gammaCorrectionShaderProgram->shaderProgram, "projectionMatrix"), 1,
        GL_FALSE, value_ptr(projectionMatrix));

    glUniformMatrix4fv(glGetUniformLocation(gammaCorrectionShaderProgram->shaderProgram, "viewMatrix"), 1, GL_FALSE,
        value_ptr(viewMatrix));

    // Set light uniforms
    glUniform3fv(glGetUniformLocation(gammaCorrectionShaderProgram->shaderProgram, "lightPositions"), 4, 
        &lightPositions[0][0]);

    glUniform3fv(glGetUniformLocation(gammaCorrectionShaderProgram->shaderProgram, "lightColors"), 4, &lightColors[0][0]);

    glUniform3f(glGetUniformLocation(gammaCorrectionShaderProgram->shaderProgram, "viewPosition"), 
        Camera::cameraPosition.x, Camera::cameraPosition.y, Camera::cameraPosition.z);

    glUniform1i(glGetUniformLocation(gammaCorrectionShaderProgram->shaderProgram, "isGammaOn"), gammaEnabled);

    // Use the floor texture now
    glBindVertexArray(planeVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gammaEnabled ? floorTextureGammaCorrected : floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    cout << (gammaEnabled ? "Gamma enabled\n" : "Gamma disabled\n");
}

