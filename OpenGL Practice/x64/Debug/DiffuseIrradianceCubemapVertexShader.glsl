#version 330 core

layout (location = 0) in vec3 position;

out vec3 worldPosition;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // Renders the cube as is and passes its local position to the fragment shader as a 3D sample vector

    worldPosition = position;
    gl_Position =  projection * view * vec4(worldPosition, 1.0);
}