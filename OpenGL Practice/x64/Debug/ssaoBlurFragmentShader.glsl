#version 330 core
out float fragColor;

in vec2 texCoords;

uniform sampler2D ssaoInput;

void main() 
{
    /* Traverse the surrounding SSAO texels between -2.0 and 2.0, sampling the SSAO texture an amount identical to the noise 
    texture’s dimensions. We offset each texture coordinate by the exact size of a single texel using textureSize that 
    returns a vec2 of the given texture’s dimensions */

    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;

    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, texCoords + offset).r;
        }
    }

    fragColor = result / (4.0 * 4.0);
}  