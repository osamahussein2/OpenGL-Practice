#version 330 core

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 BrightColor;

in SHADER_VARIABLES {
    vec3 fragPosition;
    vec3 normal;
    vec2 texCoords;
} shader_fs;

struct Light {
    vec3 position;
    vec3 color;
};

uniform Light lights[4];
uniform sampler2D diffuseTexture;
uniform vec3 viewPos;

void main()
{           
    vec3 color = texture(diffuseTexture, shader_fs.texCoords).rgb;
    vec3 normals = normalize(shader_fs.normal);

    // ambient
    vec3 ambient = 0.0 * color;

    // lighting
    vec3 lighting = vec3(0.0);
    vec3 viewDir = normalize(viewPos - shader_fs.fragPosition);

    for(int i = 0; i < 4; i++)
    {
        // Diffuse
        vec3 lightDirection = normalize(lights[i].position - shader_fs.fragPosition);

        float diff = max(dot(lightDirection, normals), 0.0);
        vec3 result = lights[i].color * diff * color;

        // Attenuation (use quadratic as gamma correction is used)
        float distance = length(shader_fs.fragPosition - lights[i].position);
        result *= 1.0 / (distance * distance);
        lighting += result;
    }

    vec3 result = ambient + lighting;

    // check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    fragColor = vec4(result, 1.0);
}