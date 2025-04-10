#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoordinates;

// Declare an interface block
out VS_OUT 
{
    vec3 FragPosition;
    vec3 Normals;
    vec2 TexCoords;
} vs_out;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
    vs_out.FragPosition = position;
    vs_out.Normals = normal;
    vs_out.TexCoords = texCoordinates;

    gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
}