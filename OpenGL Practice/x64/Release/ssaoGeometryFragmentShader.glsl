#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

in vec2 texCoords;
in vec3 FragPos;
in vec3 Normal;

void main()
{    
    /* Since SSAO is a screen-space technique where occlusion is calculated from the visible view, it makes sense to implement 
    the algorithm in view-space. Therefore, FragPos and Normal as supplied by the geometry stage’s vertex shader are 
    transformed to view space (multiplied by the view matrix as well) */

    // We should have per-fragment position and normal data available from the scene objects

    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;

    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);

    // and the diffuse per-fragment color
    gAlbedo.rgb = vec3(0.95);
}