#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoordinates;

out VS_OUT {
    vec3 FragPosition;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
    vs_out.FragPosition = position;
    vs_out.Normal = normals;
    vs_out.TexCoords = texCoordinates;

    gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
}