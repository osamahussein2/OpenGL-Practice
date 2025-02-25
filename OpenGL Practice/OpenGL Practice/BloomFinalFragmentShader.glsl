#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;

    vec3 hdrColor = texture(scene, texCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, texCoords).rgb;

    /* Add the bloom effect before applying tone mapping because then, the added brightness of bloom is also softly 
    transformed to LDR range with better relative lighting as a result */

    if(bloom)
        hdrColor += bloomColor; // additive blending
    
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    // Use gamma correction     
    result = pow(result, vec3(1.0 / gamma));

    fragColor = vec4(result, 1.0);
}