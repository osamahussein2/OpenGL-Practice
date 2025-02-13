#include "NormalMapping.h"
#include "Camera.h"

NormalMapping* NormalMapping::normalMappingInstance = 0;

// A per-fragment normal is required to get normal mapping to work

/* Normal vectors in a normal map are expressed in tangent space where normals always point roughly in the positive z 
direction. Tangent space is a space that’s local to the surface of a triangle: the normals are relative to the local 
reference frame of the individual triangles */

/* The normal map is defined in tangent space, so one way to solve the problem is to calculate a matrix to transform normals 
from tangent space to a different space such that they’re aligned with the surface’s normal direction: the normal vectors are 
then all pointing roughly in the positive y direction. The great thing about tangent space is that we can calculate this 
matrix for any type of surface so that we can properly align the tangent space’s z direction to the surface’s normal 
direction */

// This matrix is called TBN matrix where the letters depict a Tangent, Bitangent and Normal vector

/* The up vector is the surface’s normal vector. The right and forward vector are the tangent and bitangent vector 
respectively */

NormalMapping::NormalMapping() : normalMappingShaderProgram(new ShaderProgram()), quadVAO(0)
{

}

NormalMapping::~NormalMapping()
{
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteTextures(1, &diffuseMap);
    glDeleteTextures(1, &normalMap);
}

NormalMapping* NormalMapping::Instance()
{
	if (normalMappingInstance == 0)
	{
		normalMappingInstance = new NormalMapping();
		return normalMappingInstance;
	}

	return normalMappingInstance;
}

void NormalMapping::InitializeNormalMapping()
{
	glEnable(GL_DEPTH_TEST);

	normalMappingShaderProgram->InitializeShaderProgram(new VertexShaderLoader("NormalMappingVertexShader.glsl"),
		new FragmentShaderLoader("NormalMappingFragmentShader.glsl"));

    diffuseMap = LoadTexture("Textures/Brick wall.jpg");
    normalMap = LoadTexture("Textures/Brick wall normal.jpg");

    glUseProgram(normalMappingShaderProgram->shaderProgram);

    glUniform1i(glGetUniformLocation(normalMappingShaderProgram->shaderProgram, "diffuseMap"), 0);
    glUniform1i(glGetUniformLocation(normalMappingShaderProgram->shaderProgram, "normalMap"), 1);

    lightPosition = vec3(0.5f, 1.0f, 0.3f);
}

void NormalMapping::RenderNormalMapping()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // configure view/projection matrices
    mat4 projectionMatrix = perspective(radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 100.0f);
    mat4 viewMatrix = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);

    glUseProgram(normalMappingShaderProgram->shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(normalMappingShaderProgram->shaderProgram, "projectionMatrix"), 1,
        GL_FALSE, value_ptr(projectionMatrix));

    glUniformMatrix4fv(glGetUniformLocation(normalMappingShaderProgram->shaderProgram, "viewMatrix"), 1,
        GL_FALSE, value_ptr(viewMatrix));

    // Render normal-mapped quad
    glm::mat4 model = glm::mat4(1.0f);

    // Rotate the quad to show normal mapping from multiple directions
    model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));

    glUniformMatrix4fv(glGetUniformLocation(normalMappingShaderProgram->shaderProgram, "modelMatrix"), 1, 
        GL_FALSE, value_ptr(model));

    glUniform3fv(glGetUniformLocation(normalMappingShaderProgram->shaderProgram, "viewPosition"), 1, 
        value_ptr(Camera::cameraPosition));

    glUniform3fv(glGetUniformLocation(normalMappingShaderProgram->shaderProgram, "lightPosition"), 1, 
        value_ptr(lightPosition));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);

    RenderQuad();

    // Render light source (simply re-renders a smaller plane at the light's position for debugging/visualization)
    model = mat4(1.0f);
    model = translate(model, lightPosition);
    model = scale(model, glm::vec3(0.1f));

    glUniformMatrix4fv(glGetUniformLocation(normalMappingShaderProgram->shaderProgram, "modelMatrix"), 1, 
        GL_FALSE, value_ptr(model));

    RenderQuad();
}

unsigned int NormalMapping::LoadTexture(char const* path_)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path_, &width, &height, &nrComponents, 0);

    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Use GL_CLAMP_TO_EDGE to prevent semi-transparent borders, due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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

void NormalMapping::RenderQuad()
{
    // Renders a 1x1 quad in NDC with manually calculated tangent vectors

    if (quadVAO == 0)
    {
        // Positions
        glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3(1.0f, -1.0f, 0.0f);
        glm::vec3 pos4(1.0f, 1.0f, 0.0f);

        // Texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);

        // Normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // Calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;

        // Calculate the first triangle’s edges and delta UV coordinates
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        /* Pre-calculate the fractional part of the equation as f and then for each vector component we do the 
        corresponding matrix multiplication multiplied by f */
        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float quadVertices[] = {
            // positions            // normal         // texcoords  // tangent                          // bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
