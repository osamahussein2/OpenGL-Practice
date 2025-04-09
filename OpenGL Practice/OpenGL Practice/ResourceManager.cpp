#include "ResourceManager.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "stb_image.h"

// Instantiate static variables
map<string, ShaderProgram> ResourceManager::shaders;
map<string, Texture2D> ResourceManager::textures;

ShaderProgram ResourceManager::GetShader(string name)
{
    return shaders[name];
}

ShaderProgram ResourceManager::LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, string name)
{
    shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return shaders[name];
}

Texture2D ResourceManager::GetTexture(string name)
{
    return textures[name];
}

Texture2D ResourceManager::LoadTexture(const char* file, bool alpha, string name)
{
    textures[name] = loadTextureFromFile(file, alpha);
    return textures[name];
}

void ResourceManager::Clear()
{
    // Delete all shaders properly	
    for (pair<string, ShaderProgram> iter : shaders)
    {
        glDeleteProgram(iter.second.shaderProgram);
    }

    // Delete all textures properly
    for (pair<string, Texture2D> iter : textures)
    {
        glDeleteTextures(1, &iter.second.textureID);
    }
}

ShaderProgram ResourceManager::loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile)
{
    // Retrieve the vertex/fragment source code from filePath
    string vertexCode;
    string fragmentCode;
    string geometryCode;

    try
    {
        // open files
        ifstream vertexShaderFile(vShaderFile);
        ifstream fragmentShaderFile(fShaderFile);
        stringstream vShaderStream, fShaderStream;

        // read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();

        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();

        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        // if geometry shader path is present, also load a geometry shader
        if (gShaderFile != nullptr)
        {
            ifstream geometryShaderFile(gShaderFile);
            stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }

    catch (exception e)
    {
        cout << "ERROR::SHADER: Failed to read shader files" << endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str();

    // Create shader object from source code
    ShaderProgram shader;
    shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);

    return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const char* file, bool alpha)
{
    // create texture object
    Texture2D texture;
    if (alpha)
    {
        texture.internalFormat = GL_RGBA;
        texture.imageFormat = GL_RGBA;
    }
    // load image
    int width, height, nrChannels;
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);

    if (!data)
    {
        cout << "Can't find texture path at: " << file << endl;
    }

    else if (data)
    {
        // now generate texture
        texture.Generate(width, height, data);

        // and finally free image data
        stbi_image_free(data);
    }

    return texture;
}