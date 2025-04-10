#version 330 core

out vec4 fragColor;

in SHADER_VAR {
    vec3 FragPosition;
    vec2 TextureCoords;
    vec3 TangentLightPosition;
    vec3 TangentViewPosition;
    vec3 TangentFragPosition;
} shaderVar;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform float heightScale;

vec2 ParallaxMapping(vec2 textureCoords, vec3 viewDirection)
{ 
    // Parallax Mapping Part 1

    /*  Take the original texture coordinates texCoords and use these to sample the height (or depth) from the depthMap at 
    the current fragment A as H(A). Then calculate P as the x and y component of the tangent-space viewDir vector divided by 
    its z component and scaled by H(A). We also introduced a height_scale uniform for some extra control as the parallax 
    effect is usually too strong without an extra scale parameter */

    // Subtract the P vector from the texture coordinates to get the final displaced texture coordinates

    /*float height =  texture(depthMap, textureCoords).r;
    return textureCoords - viewDirection.xy * (height * heightScale);*/

    // Parallax Mapping Part 2

    // number of depth layers
    const float numberOfLayers = 10;

    // calculate the size of each layer
    float layerDepth = 1.0 / numberOfLayers;

    // depth of current layer
    float currentLayerDepth = 0.0;

    // Calculate the texture coordinate offset that we have to shift along the direction of P per layer
    vec2 P = viewDirection.xy * heightScale;
    vec2 deltaTexCoords = P / numberOfLayers;

    // get initial values
    vec2 currentTexCoords = textureCoords;

    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

    // Iterate through all the layers (start from the top), until a depthmap value is less than the layer’s depth value
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    //return currentTexCoords; // Parallax Mapping Part 2

    // Parallax Mapping Part 3

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    
    // get depth after and before collision for linear interpolation
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
    
    return finalTexCoords;

}

void main()
{           
    // offset texture coordinates with parallax mapping

    vec3 viewDirection = normalize(shaderVar.TangentFragPosition - shaderVar.TangentFragPosition);
    vec2 texCoords = shaderVar.TextureCoords;
    
    texCoords = ParallaxMapping(shaderVar.TextureCoords, viewDirection);
    
    // Make sure that the fragment gets discarded after one of the texture coordinates exceed 1 or go below 0
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0) discard;

    // Use these displaced texture coordinates as the texture coordinates for sampling the diffuse and normal map

    // obtain normal from normal map
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
   
    // get diffuse color
    vec3 color = texture(diffuseMap, texCoords).rgb;
    
    // ambient
    vec3 ambient = 0.1 * color;
    
    // diffuse
    vec3 lightDir = normalize(shaderVar.TangentLightPosition - shaderVar.TangentFragPosition);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    
    // specular    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDirection);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    fragColor = vec4(ambient + diffuse + specular, 1.0);
}