#version 330 core
out vec4 fragColor;

/* LEARNING PURPOSES ONLY - GLSL gives us the option to sample the texture image per subsample so we can create our own custom 
anti-aliasing  algorithms. To get a texture value per subsample you’d have to define the texture uniform sampler as a
sampler2DMS instead of the usual sampler2D: "uniform sampler2DMS screenTextureMS;" */

/* LEARNING PURPOSES ONLY - Using the texelFetch function it is then possible to retrieve the color value per sample: 
"vec4 colorSample = texelFetch(screenTextureMS, TexCoords, 3); // 4th sample" */

void main()
{
    fragColor = vec4(0.0, 1.0, 0.0, 1.0);
} 