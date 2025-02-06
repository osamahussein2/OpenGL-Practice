#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoordinates;

out vec2 TexCoords;

out VS_OUT {
    vec3 fragPosition;
    vec3 normal;
    vec2 texCoords;
} vs_out;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform bool reverseNormals;

void main()
{
    vs_out.fragPosition = vec3(modelMatrix * vec4(position, 1.0));

    // If we want to reverse the normals
    if (reverseNormals == true)
    {
        /* Set the normal of the out struct to make sure the outer large cube displays lighting from the inside
        instead of the default outside */
        vs_out.normal = transpose(inverse(mat3(modelMatrix))) * (-1.0 * normal);
    }

    // Else if we don't want to reverse the normals
    else if (reverseNormals == false)
    {
        // Set the normal of the out struct to make sure the outer large cube displays lighting from the outside
        vs_out.normal = transpose(inverse(mat3(modelMatrix))) * normal;
    }

    vs_out.texCoords = texCoordinates;

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}