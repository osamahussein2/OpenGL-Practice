#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light 
{
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, texCoords).rgb;
    vec3 Normal = texture(gNormal, texCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, texCoords).rgb;
    float Specular = texture(gAlbedoSpec, texCoords).a;
    
    // Then calculate lighting here
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);

    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // Calculate distance between light source and current fragment (Deferred Shading Part 2)
        float distance = length(lights[i].Position - FragPos);

        if(distance < lights[i].Radius)
        {
            // diffuse (Deferred Shading Part 1)
            vec3 lightDir = normalize(lights[i].Position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
            
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = lights[i].Color * spec * Specular;

            // attenuation
            float distance = length(lights[i].Position - FragPos);
            float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);

            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular; 
        }
    }

    fragColor = vec4(lighting, 1.0);
}