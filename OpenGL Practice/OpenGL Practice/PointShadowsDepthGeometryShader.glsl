#version 330 core

/* The geometry shader will take as input 3 triangle vertices and a uniform array of light space transformation matrices. 
The geometry shader is responsible for transforming the vertices to the light spaces. We take as input a triangle, and 
output a total of 6 triangles (6 * 3 equals 18 vertices) */

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

// FragPos from geometry shader (output per emitvertex)
out vec4 fragPosition;

void main()
{
    for(int face = 0; face < 6; face++)
    {
        /* The geometry shader has a built-in variable called gl_Layer that specifies which cubemap face to emit a 
        primitive to. When left alone, the geometry shader just sends its primitives further down the pipeline as usual, 
        but when we update this variable we can control to which cubemap face we render to for each primitive. This of 
        course only works when a cubemap texture is attached to the active framebuffer */

        gl_Layer = face;

        // Loop through each of the triangle's vertices
        for(int i = 0; i < 3; i++)
        {
            fragPosition = gl_in[i].gl_Position;

            /* Generate the output triangles by transforming each world-space input vertex to the relevant light space by 
            multiplying FragPos with the face’s light-space transformation matrix */

            gl_Position = shadowMatrices[face] * fragPosition;
            EmitVertex();
        }

        EndPrimitive();
    }
} 