#version 330 core

out vec4 fragColor;
in vec3 worldPosition;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{		
	/* The world vector acts as the normal of a tangent surface from the origin, aligned to worldPosition. Given this normal, 
    calculate all incoming radiance of the environment. The result of this radiance is the radiance of light coming from 
    Normal direction, which is what we use in the PBR shader to sample irradiance */

    vec3 N = normalize(worldPosition);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));
       
    /* sampleDelta delta value traverses the hemisphere; decreasing or increasing the sample delta will increase or decrease 
    the accuracy respectively */

    float sampleDelta = 0.025;
    float nrSamples = 0.0;

    /* Take both spherical coordinates to convert them to a 3D Cartesian sample vector, convert the sample from tangent to 
    world space oriented around the normal, and use this sample vector to directly sample the HDR environment map. We add 
    each sample result to irradiance which at the end we divide by the total number of samples taken, giving us the average 
    sampled irradiance. Note that we scale the sampled color value by cos(theta) due to the light being weaker at larger 
    angles and by sin(theta) to account for the smaller sample areas in the higher hemisphere areas */

    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));

            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    fragColor = vec4(irradiance, 1.0);
}