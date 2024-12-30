#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 TexCoordinates;

out vec2 texCoords;

void main()
{
    texCoords = TexCoordinates;
    gl_Position = vec4(position.x, position.y, 0.0, 1.0); 
}  