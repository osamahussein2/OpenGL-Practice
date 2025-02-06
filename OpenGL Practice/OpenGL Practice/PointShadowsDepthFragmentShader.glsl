#version 330 core

in vec4 fragPosition;

uniform vec3 lightPosition;
uniform float farPlane;

void main()
{
    float lightDistance = length(fragPosition.xyz - lightPosition);
    
    // Map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / farPlane;
    
    // Write this as modified depth
    gl_FragDepth = lightDistance;
}