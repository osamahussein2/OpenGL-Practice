#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 modelMatrix;

void main()
{
    // Transform the vertices to world-space and send them to the geometry shader
    gl_Position = modelMatrix * vec4(position, 1.0);
}