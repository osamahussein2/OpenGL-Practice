#include "ParallaxMapping.h"
#include "Camera.h"

ParallaxMapping* ParallaxMapping::parallaxMappingInstance = 0;

/* Parallax mapping is closely related to the family of displacement mapping techniques that displace or offset vertices 
based on geometrical information stored inside a texture. One way to do this, is to take a plane with roughly 1000 vertices 
and displace each of these vertices based on a value in a texture that tells us the height of the plane at that specific 
area. Such a texture that contains height values per texel is called a height map */

/* When spanned over a plane, each vertex is displaced based on the sampled height value in the height map, transforming a 
flat plane to a rough bumpy surface based on a material’s geometric properties */

/* The idea behind parallax mapping is to alter the texture coordinates in such a way that it looks like a fragment’s surface 
is higher or lower than it actually is, all based on the view direction and a heightmap */

/* By transforming the fragment-to-view direction vector V¯ to tangent space, the transformed P vector will have its x and y 
component aligned to the surface’s tangent and bitangent vectors. As the tangent and bitangent vectors are pointing in the 
same direction as the surface’s texture coordinates we can take the x and y components of P vector as the texture coordinate 
offset, regardless of the surface’s orientation */

ParallaxMapping::ParallaxMapping() : parallaxMappingShaderProgram(new ShaderProgram()), diffuseMap(NULL), normalMap(NULL),
heightMap(NULL), lightPosition(NULL), quadVAO(NULL), quadVBO(NULL), heightScale(0.1f)
{
}

ParallaxMapping::~ParallaxMapping()
{
    parallaxMappingInstance = NULL;

    array<unsigned int, 3> textureMaps;
    textureMaps = { diffuseMap, normalMap, heightMap };

    for (int i = 0; i < textureMaps.size(); i++)
    {
        glDeleteTextures(1, &textureMaps[i]);
    }

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
}

ParallaxMapping* ParallaxMapping::Instance()
{
	if (parallaxMappingInstance == 0)
	{
		parallaxMappingInstance = new ParallaxMapping();
	}

	return parallaxMappingInstance;
}

void ParallaxMapping::InitializeParallaxMapping()
{
	glEnable(GL_DEPTH_TEST);

	parallaxMappingShaderProgram->InitializeShaderProgram(new VertexShaderLoader("ParallaxMappingVertexShader.glsl"),
		new FragmentShaderLoader("ParallaxMappingFragmentShader.glsl"));

    // Parallax Mapping Part 1
    /*diffuseMap = LoadTexture("Textures/bricks2.jpg");
    normalMap = LoadTexture("Textures/bricks2_normal.jpg");
    heightMap = LoadTexture("Textures/bricks2_disp.jpg");*/

    // Parallax Mapping Part 2
    diffuseMap = LoadTexture("Textures/Wood.png");
    normalMap = LoadTexture("Textures/toy_box_normal.png");
    heightMap = LoadTexture("Textures/toy_box_disp.png");

    glUseProgram(parallaxMappingShaderProgram->shaderProgram);

    glUniform1i(glGetUniformLocation(parallaxMappingShaderProgram->shaderProgram, "diffuseMap"), 0);
    glUniform1i(glGetUniformLocation(parallaxMappingShaderProgram->shaderProgram, "normalMap"), 1);
    glUniform1i(glGetUniformLocation(parallaxMappingShaderProgram->shaderProgram, "heightMap"), 2);

    lightPosition = vec3(0.5f, 1.0f, 0.3f);
}

void ParallaxMapping::RenderParallaxMapping()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 projection = perspective(radians(Camera::fieldOfView), float(1280 / 960), 0.1f, 100.0f);
    mat4 view = lookAt(Camera::cameraPosition, Camera::cameraPosition + Camera::cameraFront, Camera::cameraUp);

    glUseProgram(parallaxMappingShaderProgram->shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(parallaxMappingShaderProgram->shaderProgram, "projectionMatrix"), 
        1, GL_FALSE, value_ptr(projection));

    glUniformMatrix4fv(glGetUniformLocation(parallaxMappingShaderProgram->shaderProgram, "viewMatrix"), 1, 
        GL_FALSE, value_ptr(view));

    // Render a parallax-mapped quad
    mat4 model = mat4(1.0f);

    // Rotate the quad to show parallax mapping from multiple directions
    model = rotate(model, radians((float)glfwGetTime() * -10.0f), normalize(vec3(1.0, 0.0, 1.0)));

    glUniformMatrix4fv(glGetUniformLocation(parallaxMappingShaderProgram->shaderProgram, "modelMatrix"), 1, 
        GL_FALSE, value_ptr(model));

    glUniform3f(glGetUniformLocation(parallaxMappingShaderProgram->shaderProgram, "viewPosition"), Camera::cameraPosition.x,
        Camera::cameraPosition.y, Camera::cameraPosition.z);

    glUniform3f(glGetUniformLocation(parallaxMappingShaderProgram->shaderProgram, "lightPosition"), lightPosition.x,
        lightPosition.y, lightPosition.z);

    // adjust with Q and E keys
    glUniform1f(glGetUniformLocation(parallaxMappingShaderProgram->shaderProgram, "heightScale"), heightScale);

    std::cout << heightScale << std::endl;

    // Set the texture maps to their respective texture values
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, heightMap);

    RenderQuad();

    // Render light source (simply re-renders a smaller plane at the light's position for debugging/visualization)
    model = mat4(1.0f);
    model = translate(model, lightPosition);
    model = scale(model, vec3(0.1f));

    glUniformMatrix4fv(glGetUniformLocation(parallaxMappingShaderProgram->shaderProgram, "modelMatrix"), 1, 
        GL_FALSE, value_ptr(model));

    RenderQuad();
}

void ParallaxMapping::RenderQuad()
{
    if (quadVAO == 0)
    {
        // positions
        vec3 pos1(-1.0f, 1.0f, 0.0f);
        vec3 pos2(-1.0f, -1.0f, 0.0f);
        vec3 pos3(1.0f, -1.0f, 0.0f);
        vec3 pos4(1.0f, 1.0f, 0.0f);
        // texture coordinates
        vec2 uv1(0.0f, 1.0f);
        vec2 uv2(0.0f, 0.0f);
        vec2 uv3(1.0f, 0.0f);
        vec2 uv4(1.0f, 1.0f);
        // normal vector
        vec3 normal(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        vec3 tangent1, bitangent1;
        vec3 tangent2, bitangent2;

        // triangle 1
        vec3 edge1 = pos2 - pos1;
        vec3 edge2 = pos3 - pos1;
        vec2 deltaUV1 = uv2 - uv1;
        vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = normalize(bitangent1);

        // triangle 2
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = normalize(bitangent2);


        array<float, 84> quadVertices = 
        {
            // positions            // normal         // texcoords  // tangent                          // bitangent
            pos1.x, pos1.y, pos1.z, normal.x, normal.y, normal.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, normal.x, normal.y, normal.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, normal.x, normal.y, normal.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, normal.x, normal.y, normal.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, normal.x, normal.y, normal.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, normal.x, normal.y, normal.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
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

        glEnableVertexAttribArray(3); // tangent attribute
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));

        glEnableVertexAttribArray(4); // bi tangent attribute
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned int ParallaxMapping::LoadTexture(char const* path_)
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    else
    {
        glDeleteTextures(1, &textureID);

        std::cout << "Texture failed to load at path: " << path_ << std::endl;
        stbi_image_free(data);
    }

	return textureID;
}