/* 
At the start of a geometry shader we need to declare the type of primitive input we’re receiving from the vertex shader. 
We do this by declaring a layout specifier in front of the in keyword. This input layout qualifier can take any of the 
following primitive values: 

	- points: when drawing GL_POINTS primitives (1).
	- lines: when drawing GL_LINES or GL_LINE_STRIP (2).
	- lines_adjacency: GL_LINES_ADJACENCY or GL_LINE_STRIP_ADJACENCY (4).
	- triangles: GL_TRIANGLES, GL_TRIANGLE_STRIP or GL_TRIANGLE_FAN (3).
	- triangles_adjacency : GL_TRIANGLES_ADJACENCY or GL_TRIANGLE_STRIP_ADJACENCY (6)

We also need to specify a primitive type that the geometry shader will output and we do this via a layout specifier in 
front of the out keyword. Like the input layout qualifier, the output layout qualifier can take several primitive values:

	- points
	- line_strip
	- triangle_strip 
*/

#version 330 core

//layout (points) in;
//layout (triangle_strip, max_vertices = 5) out;

/*layout (points) in;
layout (line_strip, max_vertices = 2) out; // LEARNING PURPOSES ONLY */

//layout (triangles) in;
//layout (triangle_strip, max_vertices = 3) out;

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    //vec3 color;
	//vec2 texCoords;
	vec3 normal;

} gs_in[];

//out vec2 TexCoords;

//out vec3 fragmentColor;

//uniform float time;

const float MAGNITUDE = 0.4;

uniform mat4 projectionMatrix;

// Geometry Shader Part 1
void BuildHouse(vec4 position)
{    
	/* Using the vertex data from the vertex shader stage we can generate new data with 2 geometry shader functions called 
	EmitVertex and EndPrimitive. The geometry shader expects you to generate/output at least one of the primitives you 
	specified as output */

    //fragmentColor = gs_in[0].color; // gs_in[0] since there's only one input vertex

    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0); //bottom-left   
    EmitVertex();

    gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0); // bottom-right
    EmitVertex();

    gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0); // top-left
    EmitVertex();

    gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0); // top-right
    EmitVertex();

    gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0); // top
    //fragmentColor = vec3(1.0, 1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}

// Geometry Shader Part 2
vec3 GetNormal()
{
	/* Here we retrieve two vectors a and b that are parallel to the surface of the triangle using vector subtraction. 
	Subtracting two vectors from each other results in a vector that is the difference of the two vectors. Since all 3 points
	lie on the triangle plane, subtracting any of its vectors from each other results in a vector parallel to the plane. 
	If we switched a and b in the cross function we’d get a normal vector that points in the opposite direction */

	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(a, b));
}

/*vec4 explode(vec4 position, vec3 normal)
{
	/* Using a normal vector, we can create an explode function that takes this normal vector along with a vertex position
	vector. The function returns a new vector that translates the position vector along the direction of the normal vector */

	//float magnitude = 2.0;
	//vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
	//return position + vec4(direction, 0.0);
//}

// Geometry Shader Part 3
void GenerateLine(int index)
{
	/* The transformed view-space normal vector is then passed to the next shader stage via an interface
	block. The geometry shader then takes each vertex (with a position and a normal vector) and draws a normal vector from 
	each position vector: */ 

	gl_Position = projectionMatrix * gl_in[index].gl_Position;
	EmitVertex();
	
	gl_Position = projectionMatrix * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
	EmitVertex();
	
	EndPrimitive();
}

void main()
{
	// Geometry Shader Part 1
	//BuildHouse(gl_in[0].gl_Position);

	// Geometry Shader Part 2
	/*vec3 normal = GetNormal();

	gl_Position = explode(gl_in[0].gl_Position, normal);
	TexCoords = gs_in[0].texCoords;
	EmitVertex();

	gl_Position = explode(gl_in[1].gl_Position, normal);
	TexCoords = gs_in[1].texCoords;
	EmitVertex();

	gl_Position = explode(gl_in[2].gl_Position, normal);
	TexCoords = gs_in[2].texCoords;
	EmitVertex();

	EndPrimitive();*/

	// Geometry Shader Part 3
	GenerateLine(0); // first vertex normal
	GenerateLine(1); // second vertex normal
	GenerateLine(2); // third vertex normal

	/* LEARNING PURPOSES ONLY
	
	gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4( 0.1, 0.0, 0.0, 0.0);
	EmitVertex();

	EndPrimitive(); */
}