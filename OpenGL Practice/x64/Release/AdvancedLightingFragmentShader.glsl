#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPosition;
    vec3 Normals;
    vec2 TexCoords;
} fs_in;

uniform sampler2D floorTexture;
uniform vec3 lightPosition;
uniform vec3 viewPosition;
uniform bool includeBlinnShading;

/* 
	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir); // Only available for Blinn shading
*/

void main()
{           
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;

    // Ambient
    vec3 ambient = 0.05 * color;

    // Diffuse
    vec3 lightDir = normalize(lightPosition - fs_in.FragPosition);
    vec3 normal = normalize(fs_in.Normals);

    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    // Specular
    vec3 viewDir = normalize(viewPosition - fs_in.FragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;

    if(includeBlinnShading)
    {
        vec3 halfwayDirection = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDirection), 0.0), 32.0);
    }

    else if (!includeBlinnShading)
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }

    vec3 specular = vec3(0.3) * spec; // assuming bright white light color
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}