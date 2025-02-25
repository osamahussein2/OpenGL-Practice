#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 textureCoords;

out SHADER_VARIABLES {
    vec3 fragPosition;
    vec3 normal;
    vec2 texCoords;
} shader_vs;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    shader_vs.fragPosition = vec3(model * vec4(position, 1.0));   
    shader_vs.texCoords = textureCoords;
        
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    shader_vs.normal = normalize(normalMatrix * normals);
    
    gl_Position = projection * view * model * vec4(position, 1.0);
}