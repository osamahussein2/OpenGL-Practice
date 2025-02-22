#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 biTangent;

out SHADER_VAR {
    vec3 FragPosition;
    vec2 TextureCoords;
    vec3 TangentLightPosition;
    vec3 TangentViewPosition;
    vec3 TangentFragPosition;
} shaderVar;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform vec3 lightPosition;
uniform vec3 viewPosition;

void main()
{
    shaderVar.FragPosition = vec3(modelMatrix * vec4(position, 1.0));   
    shaderVar.TextureCoords = texCoords;   
    
    vec3 T = normalize(mat3(modelMatrix) * tangent);
    vec3 B = normalize(mat3(modelMatrix) * biTangent);
    vec3 N = normalize(mat3(modelMatrix) * normals);

    mat3 TBN = transpose(mat3(T, B, N));

    shaderVar.TangentLightPosition = TBN * lightPosition;
    shaderVar.TangentViewPosition  = TBN * viewPosition;
    shaderVar.TangentFragPosition  = TBN * shaderVar.FragPosition;
    
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}