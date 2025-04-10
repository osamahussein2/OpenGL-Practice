#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
};
uniform Light light;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, texCoords).rgb;
    vec3 Normal = texture(gNormal, texCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, texCoords).rgb;
    float AmbientOcclusion = texture(ssao, texCoords).r;
    
    /* To apply the occlusion factors to the lighting equation: multiply the per-fragment ambient occlusion factor to the 
    lighting’s ambient component */

    vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
    vec3 lighting  = ambient; 
    vec3 viewDir  = normalize(-FragPos); // viewpos is (0.0.0)

    // diffuse
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;

    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.Color * spec;

    // attenuation
    float distance = length(light.Position - FragPos);
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);

    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    fragColor = vec4(lighting, 1.0);
}