#pragma once

#include <glad/glad.h>
#include <cstddef> // so that we can set a non-pointer object to NULL

#ifndef TEXTURE_H
#define TEXTURE_H

class Texture2D
{
public:

    // Constructor
    Texture2D();

    // Generates texture from image data
    void Generate(unsigned int width_, unsigned int height_, unsigned char* data);

    // Binds the texture as the current active GL_TEXTURE_2D texture object
    void Bind() const;

    // Holds the ID of the texture object, used for all texture operations to reference to any textures initialized
    unsigned int textureID;

    // Texture image dimensions (width and height of loaded image in pixels)
    unsigned int width, height;

    unsigned int internalFormat; // format of texture object
    unsigned int imageFormat; // format of loaded image

    unsigned int wrapS; // wrapping mode on S axis
    unsigned int wrapT; // wrapping mode on T axis

    unsigned int filterMin; // filtering mode if texture pixels < screen pixels
    unsigned int filterMax; // filtering mode if texture pixels > screen pixels
};

#endif TEXTURE_H
