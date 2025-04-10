#version 330 core

out vec4 fragColor;

in vec3 worldPosition;

uniform samplerCube environmentMap;

// Also applies to Specular IBL as well

void main()
{
    //vec3 envColor = texture(environmentMap, worldPosition).rgb;

    // Specular IBL

    /* When using the pre-filtered environment cubemap in the skybox shader and forcefully sample somewhat above its first 
    mip level like so: the result will look like a blurrier version of the original environment */

    //vec3 envColor = textureLod(environmentMap, worldPosition, 1.2).rgb;

    vec3 envColor = textureLod(environmentMap, worldPosition, 0.0).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    fragColor = vec4(envColor, 1.0);
}