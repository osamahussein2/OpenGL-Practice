#pragma once
#include "ShaderProgram.h"
#include <vector>

using namespace std;
using namespace glm;

struct Vertex
{
	vec3 meshPosition, meshNormal;
	vec2 meshTextureCoordinates;
};

struct Texture
{
	unsigned int textureID;
	string textureType;
	string texturePath;
};

class Mesh
{
public:
	Mesh(vector<Vertex> vertices_, vector<unsigned int> indices_, vector<Texture> textures_);
	void SetupMesh();
	void DrawMesh(ShaderProgram *shaderProgram_);

private:
	// Create a vector of mesh data information here
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	// Create render data information here
	unsigned int VAO, VBO, EBO;

	// Create diffuse and specular textures number here to use the texture diffuse and specular uniforms in the shader
	unsigned int diffuseNumber, specularNumber;
};