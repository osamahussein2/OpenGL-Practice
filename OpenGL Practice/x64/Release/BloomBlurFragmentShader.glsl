#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D image;

uniform bool horizontal;

/* Take a relatively small sample of Gaussian weights that we each use to assign a specific weight to the horizontal or 
vertical samples around the current fragment */

uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
    /* Base the offset distance on the exact size of a texel obtained by the division of 1.0 over the size of the texture 
    (a vec2 from textureSize) */
    
    vec2 texOffset = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, texCoords).rgb * weight[0];
    
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, texCoords + vec2(texOffset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, texCoords - vec2(texOffset.x * i, 0.0)).rgb * weight[i];
        }
    }

    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, texCoords + vec2(0.0, texOffset.y * i)).rgb * weight[i];
            result += texture(image, texCoords - vec2(0.0, texOffset.y * i)).rgb * weight[i];
        }
    }
    
    fragColor = vec4(result, 1.0);
}