#version 330 core

out vec4 fragColor;

in VS_OUT {
    vec3 FragPosition;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D floorTexture;

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 viewPosition;

uniform bool isGammaOn;

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
    // Diffuse lighting
    vec3 lightDirection = normalize(lightPos - fragPos);
    float diffValue = max(dot(lightDirection, normal), 0.0);
    vec3 diffuse = diffValue * lightColor;

    // Specular lighting
    vec3 viewDirection = normalize(viewPosition - fragPos);
    vec3 reflectDirection = reflect(-lightDirection, normal);

    float spec = 0.0;
    vec3 halfwayDirection = normalize(lightDirection + viewDirection);  
    spec = pow(max(dot(normal, halfwayDirection), 0.0), 64.0);
    vec3 specular = spec * lightColor;    

    // Using attenuation
    float max_distance = 1.5;
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (isGammaOn ? distance * distance : distance);
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    return diffuse + specular;
}

void main()
{           
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;

    vec3 lighting = vec3(0.0);
    for(int i = 0; i < 4; ++i)
    {
        lighting += BlinnPhong(normalize(fs_in.Normal), fs_in.FragPosition, lightPositions[i], lightColors[i]);
    }

    color *= lighting;

    if(isGammaOn == true)
    {
        color = pow(color, vec3(1.0/2.2));
    }

    fragColor = vec4(color, 1.0);
}