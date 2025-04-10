#version 330 core

out float fragColor;

in vec2 texCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// Parameters
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(1280.0/4.0, 960.0/4.0); 

uniform mat4 projection;

void main()
{
    /* We want to tile the noise texture all over the screen, but as the TexCoords vary between 0.0 and 1.0, the texNoise 
    texture won’t tile at all. So we’ll calculate the required amount to scale TexCoords by dividing the screen’s dimensions 
    by the noise texture size */

    // get input for SSAO algorithm
    vec3 fragPos = texture(gPosition, texCoords).xyz;
    vec3 normal = normalize(texture(gNormal, texCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, texCoords * noiseScale).xyz);

    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    /* Iterate over each of the kernel samples, transform the samples from tangent to viewspace, add them to the current 
    fragment position, and compare the fragment position’s depth with the sample depth stored in the view-space position 
    buffer (calculate occlusion factor) */

    float occlusion = 0.0;

    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        /* We want to transform sample to screen-space so we can sample the position/depth value of sample as if we were 
        rendering its position directly to the screen. As the vector is currently in view-space, we’ll transform it to 
        clip-space first using the projection matrix uniform */

        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample
        
        /* We use the offset vector’s x and y component to sample the position texture to retrieve the depth (or z value) of 
        the sample position as seen from the viewer’s perspective (the first non-occluded visible fragment). We then check if 
        the sample’s current depth value is larger than the stored depth value and if so, we add to the final contribution 
        factor */

        /* GLSL smoothstep function smoothly interpolates its third parameter between the first and second parameter’s range, 
        returning 0.0 if less than or equal to its first parameter and 1.0 if equal or higher to its second parameter. If the 
        depth difference ends up between radius, its value gets smoothly interpolated between 0.0 and 1.0 by the following
        curve */

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }

    /* Normalize the occlusion contribution by the size of the kernel and output the results. Note that we subtract the 
    occlusion factor from 1.0 so we can directly use the occlusion factor to scale the ambient lighting component */

    occlusion = 1.0 - (occlusion / kernelSize);
    
    fragColor = occlusion;
}