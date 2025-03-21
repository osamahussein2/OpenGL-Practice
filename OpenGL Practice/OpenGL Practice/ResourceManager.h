#pragma once

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <string>

#include <glad/glad.h>

#include "Texture2D.h"
#include "ShaderProgram.h"

class ResourceManager
{
public:

    // resource storage
    static map<string, ShaderProgram> shaders;
    static map<string, Texture2D> textures;

    // Loads (and generates) a shader program from file loading vertex, fragment (and geometry) shader's source code.
    // If gShaderFile is not nullptr, it also loads a geometry shader
    static ShaderProgram LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, string name);
    
    // Retrieves a stored sader
    static ShaderProgram GetShader(string name);

    // Loads (and generates) a texture from file
    static Texture2D LoadTexture(const char* file, bool alpha, string name);

    // Retrieves a stored texture
    static Texture2D GetTexture(string name);

    // Properly de-allocates all loaded resources
    static void Clear();

private:

    // private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
    ResourceManager() { }

    // loads and generates a shader from file
    static ShaderProgram loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile = nullptr);

    // loads a single texture from file
    static Texture2D loadTextureFromFile(const char* file, bool alpha);
};

#endif RESOURCE_MANAGER_H