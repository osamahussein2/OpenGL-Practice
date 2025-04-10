#version 330 core

out vec4 fragColor;

in VS_OUT {
    vec3 fragPosition;
    vec3 normal;
    vec2 texCoords;
} fs_in;

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;

uniform vec3 lightPosition;
uniform vec3 viewPosition;

uniform float farPlane;
uniform bool shadows;

// Array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 fragPosition)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPosition - lightPosition;

    // Use the fragment to light vector to sample from the depth map (Point Shadows Part 1)  
    //float closestDepth = texture(depthMap, fragToLight).r;

    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    //closestDepth *= farPlane;

    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);

    // Test for shadows (Point Shadows Part 1)
    // Use a much larger bias since depth is now in [near plane, far plane] range
    //float bias = 0.05;
    //float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0; 
    
    // Display closestDepth as debug so that it can visualize the depth cubemap
    // fragColor = vec4(vec3(closestDepth / farPlane), 1.0);
    
    // Point Shadows Part 2

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPosition - fragPosition);
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;

    for(int i = 0; i < samples; ++i)
    {
        /* Add multiple offsets, scaled by some diskRadius, around the original fragToLight direction vector to sample 
        from the cubemap */

        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= farPlane;   // undo mapping [0;1]

        if(currentDepth - bias > closestDepth)
        {
            shadow += 1.0;
        }
    }

    shadow /= float(samples);
        
    return shadow;
}

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.texCoords).rgb;
    vec3 normal = normalize(fs_in.normal);
    vec3 lightColor = vec3(0.3);

    // ambient
    vec3 ambient = 0.3 * lightColor;

    // diffuse
    vec3 lightDirection = normalize(lightPosition - fs_in.fragPosition);
    float diff = max(dot(lightDirection, normal), 0.0);

    vec3 diffuse = diff * lightColor;

    // specular
    vec3 viewDirection = normalize(viewPosition - fs_in.fragPosition);
    vec3 reflectDirection = reflect(-lightDirection, normal);

    float spec = 0.0;

    vec3 halfwayDirection = normalize(lightDirection + viewDirection);  
    spec = pow(max(dot(normal, halfwayDirection), 0.0), 64.0);
    vec3 specular = spec * lightColor;   
    
    // calculate shadow
    float shadow = shadows ? ShadowCalculation(fs_in.fragPosition) : 0.0;                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    fragColor = vec4(lighting, 1.0);
}