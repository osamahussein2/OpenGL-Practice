#include "AdvancedLighting.h"
#include "Camera.h"

AdvancedLighting* AdvancedLighting::advancedLightingInstance = 0;

/* Instead of relying on a reflection vector in Phong shading, a so called halfway vector that is a unit vector exactly 
halfway between the view direction and the light direction. The closer this halfway vector aligns with the surface’s normal 
vector, the higher the specular contribution (Blinn shading) */

/* When the view direction is perfectly aligned with the (now imaginary) reflection vector, the halfway vector aligns 
perfectly with the normal vector. The closer the view direction is to the original reflection direction, the stronger the 
specular highlight */

/* Getting the halfway vector is easy, we add the light’s direction vector and view vector together and normalize the result:

Halfway vector = light direction vector + view vector / || light direction vector + view vector ||

 */

AdvancedLighting::AdvancedLighting() : advancedLightingShaderProgram(new ShaderProgram()), planeVertices({NULL}), 
isBlinnOn(false)
{

}

AdvancedLighting::~AdvancedLighting()
{
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
}

AdvancedLighting* AdvancedLighting::Instance()
{
	if (advancedLightingInstance == 0)
	{
		advancedLightingInstance = new AdvancedLighting();
		return advancedLightingInstance;
	}

	return advancedLightingInstance;
}

void AdvancedLighting::InitializeVertices()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    advancedLightingShaderProgram->InitializeShaderProgram(new VertexShaderLoader("AdvancedLightingVertexShader.glsl"),
        new FragmentShaderLoader("AdvancedLightingFragmentShader.glsl"));

    planeVertices = 
    {
        // Positions            // Normals         // texture coordinates (vec2)
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

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void AdvancedLighting::InitializeTextures()
{
    glGenTextures(1, &floorTexture);

    int width, height, nrComponents;

    unsigned char* data = stbi_load("Textures/Wood.png", &width, &height, &nrComponents, 0);

    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, floorTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. 
        // Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << "Textures/Wood.png" << std::endl;
        stbi_image_free(data);
    }

    // Use the shader program for the advanced lighting shaders to set the uniform texture
    glUseProgram(advancedLightingShaderProgram->shaderProgram);
    glUniform1i(glGetUniformLocation(advancedLightingShaderProgram->shaderProgram, "floorTexture"), 0);

    // Lighting info
    lightPosition = vec3(0.0f, 0.0f, 0.0f);
}

void AdvancedLighting::SetUpAdvancedLighting()
{
    // Draw objects and set up some advanced lighting
    glUseProgram(advancedLightingShaderProgram->shaderProgram);

    glm::mat4 projection = perspective(glm::radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 100.0f);
    glm::mat4 view = Camera::CameraLookAt();

    glUniformMatrix4fv(glGetUniformLocation(advancedLightingShaderProgram->shaderProgram, "projectionMatrix"), 1, 
        GL_FALSE, value_ptr(projection));

    glUniformMatrix4fv(glGetUniformLocation(advancedLightingShaderProgram->shaderProgram, "viewMatrix"), 1,
        GL_FALSE, value_ptr(view));

    // Set some lighting uniforms
    glUniform3fv(glGetUniformLocation(advancedLightingShaderProgram->shaderProgram, "viewPosition"), 1, 
        value_ptr(Camera::cameraPosition));

    glUniform3fv(glGetUniformLocation(advancedLightingShaderProgram->shaderProgram, "lightPosition"), 1, 
        value_ptr(lightPosition));

    glUniform1i(glGetUniformLocation(advancedLightingShaderProgram->shaderProgram, "includeBlinnShading"), isBlinnOn);

    // Render the floor texture after setting it up
    glBindVertexArray(planeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    std::cout << (isBlinnOn ? "Blinn-Phong" : "Phong") << std::endl;
}
