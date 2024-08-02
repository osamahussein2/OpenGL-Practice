#include "Texture.h"

Texture::Texture()
{

}

Texture::~Texture()
{

}

void Texture::InitializeTexture()
{
	// Texture coordinates in OpenGL go from 0,0 (bottom left) to 1,1 (top right)

	float textureCoordinates[] = {
		0.0f, 0.0f, // bottom left corner of the OpenGL window
		1.0f, 0.0f, // bottom right corner of the OpenGL window
		0.5f, 1.0f // top center of the OpenGL window
	};

	// Retrieving the texture colors using texture coordinates is called sampling
	// Texture sampling has a loose interpretation and we can sample the texture in different ways
	// To accomplish it, one must tell OpenGL how to sample the textures

	// Types of behaviour of texture wrapping in OpenGL: GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE and
	// GL_CLAMP_TO_BORDER

	/* GL_REPEAT is the default texture behaviour that repeats the textured image
	
	GL_MIRRORED_REPEAT is similar to GL_REPEAT except the textures will mirror the image on each repeat

	GL_CLAMP_TO_EDGE will clamp the texture image between 0 and 1. The result of higher texture coordinates will
	be clamped to the edge, which results in the image having a stretched edge pattern (makes the image look bigger)

	GL_CLAMP_TO_BORDER basically are texture coordinates that are out of range gives the user an option
	to specify a border color to clamp the texture image? */

	// The texture coordinates consists of a s, t and r (only for 3D textures) axes

	// First parameter passes in the texture target, second parameter passes in the axes that we want to set
	// And the third parameter passes in what we want the texture wrapping mode to be
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	// If we choose GL_CLAMP_TO_BORDER, we should also specify a border color (r, g, b, a)
	float borderColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Texture flitering includes GL_NEAREST and GL_LINEAR

	/* GL_NEAREST selects the texture pixel that center is closest to the texture coordinate (e.g. closest to 0, 0.5)
	
	GL_LINEAR takes an interpolated value from the texture coordinate's neighouring texture pixels, approximating
	a color between the texture pixels (or texels) 
	
	The smaller the distance from the texture coordinate to a texel's center, the more that texel's color contributes
	to the sampled color */

	/* GL_NEAREST results in a blocked pattern where we can clearly see the pixels that form the texture (produces a 
	8-bit look that some developers prefer) compared to GL_LINEAR resulting in a more smooth pattern where the each
	pixel is less visible (produces a realistic output) */

	// Texture filtering can be set for magnifying (scaling up) and minifying (scaling downward) operations

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Nearest filtering for downward scaled textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear filtering for upscale textures

	// Mipmaps are collections of texture images where each subsequent (following) texture is twice as small compared
	// to the previous one

	/* After a certain distance threshold from the viewer, OpenGL will use a different mipmap texture that best suits
	the distance to the object. Because the object is far away, the object won't be visible to the user. OpenGL then
	samples the correct texture pixels and there is less cache memory involved when sampling that part of the mipmaps */

	// The filtering method between mipmap levels consist of GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST,
	// GL_NEAREST_MIPMAP_LINEAR and GL_LINEAR_MIPMAP_LINEAR

	/* GL_NEAREST_MIPMAP_NEAREST takes the nearest mipmap to match the pixel size and uses the nearest neighbor
	interpolation for texture sampling
	
	GL_LINEAR_MIPMAP_NEAREST takes the nearest mipmap level and samples that level using linear interpolation

	GL_NEAREST_MIPMAP_LINEAR linearly interpolates between the 2 mipmaps that most closely match the size of a pixel
	and samples the interpolated level via nearest neighbor interpolation

	GL_LINEAR_MIPMAP_LINEAR linearly interpolates between the 2 closest mipmaps and samples the interpolated level 
	via nearest neighbor interpolation */

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Setting the mipmap filter on a magnifying fliter has no effect on the mipmaps since they are primarily used
	when downscaling the textures as in texture magnification doesn't use mipmaps and passing in a mipmapping
	filtering option will give an OpenGL INVALID_CODE_ENUM error code */


}