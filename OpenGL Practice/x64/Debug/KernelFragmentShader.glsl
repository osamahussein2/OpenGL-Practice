#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D textureImage;

const float offset = 1.0 / 300.0;

vec2 offsets[9];
float kernel[9];
vec3 sampleTexture[9];

vec3 kernelTexture;

float[9] SharpenKernel()
{
	/* Define the kernel array, which in this case is a sharpen kernel that sharpens each color value by sampling
	all surrounding pixels in an interesting way */

	kernel = float[]
	(
		-1, -1, -1,
		-1, 9, -1,
		-1, -1, -1
	);

	return kernel;
}

float[9] KernelBlurEffect()
{
	kernel = float[]
	(
		1.0 / 16, 2.0 / 16, 1.0 / 16,
		2.0 / 16, 4.0 / 16, 2.0 / 16,
		1.0 / 16, 2.0 / 16, 1.0 / 16
	);

	return kernel;
}

float[9] EdgeDetectionKernel()
{
	kernel = float[]
	(
		1.0, 1.0, 1.0,
		1.0, -8.0, 1.0,
		1.0, 1.0, 1.0
	);

	return kernel;
}

void main()
{
	/* A kernel (or convolution matrix) is a small matrix-like array of values centered on the current pixel that
	multiplies surrounding pixel values by its kernel values and adds them together to form a single value. We assume
	each kernel we'll be using is 3x3 kernel (which most kernels are)
	*/
	
	// Create offsets array for each surrounding texture coordinate (it's a constant value)
	offsets = vec2[]
	(
		vec2(-offset, offset), // top-left
		vec2( 0.0f, offset), // top-center
		vec2( offset, offset), // top-right
		vec2(-offset, 0.0f), // center-left
		vec2( 0.0f, 0.0f), // center-center
		vec2( offset, 0.0f), // center-right
		vec2(-offset, -offset), // bottom-left
		vec2( 0.0f, -offset), // bottom-center
		vec2( offset, -offset) // bottom-right
	);

	// Use the sharpen kernel effect function
	//SharpenKernel();

	// Use the blur kernel effect function
	//KernelBlurEffect();

	// Detect edges by highlighting them and darken the rest (useful for caring about edges in an image)
	EdgeDetectionKernel();

	// Add each offset to the current texture coordinate when sampling
	for(int i = 0; i < sampleTexture.length(); i++)
	{
		sampleTexture[i] = vec3(texture(textureImage, texCoords.st + offsets[i]));
	}
	
	kernelTexture = vec3(0.0);

	// Multiply these texture values with the weighted kernel values that are added up together
	for(int i = 0; i < sampleTexture.length(); i++)
	{
		kernelTexture += sampleTexture[i] * kernel[i];
	}

	fragColor = vec4(kernelTexture, 1.0);
}
