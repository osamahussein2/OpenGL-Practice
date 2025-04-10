#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 textureCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform bool inverse_normals;

void main()
{
    vs_out.FragPos = vec3(model * vec4(position, 1.0));   
    vs_out.TexCoords = textureCoords;
    
    vec3 n = inverse_normals ? -normals : normals;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalize(normalMatrix * n);
    
    gl_Position = projection * view * model * vec4(position, 1.0);
}