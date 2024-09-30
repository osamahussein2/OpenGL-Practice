#pragma once

#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"

// Include assimp library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;

// Makes it easier to access Assimp's library without having to declare it every time I use it
using namespace Assimp;

class Model
{
public:
	Model(const char* filePath_);
	~Model();

	void DrawModel();
private:

	void LoadModel(string filePath_);
	void ProcessNode(aiNode* node_, const aiScene* scene_);
	Mesh* ProcessMesh(aiMesh* mesh_, const aiScene* scene_);

	vector<Texture> LoadMaterialTexture(aiMaterial* material_, aiTextureType textureType, string typeName_);

	unsigned int TextureFromFile(const char* filePath_, const string& fileDirectory_);

	unsigned int textureID;
	int width, height, nrComponents;

	unsigned char* data;

	// Model data
	vector<Mesh*> meshes;
	string fileDirectory;

	// Let's use Assimp's library here
	Importer assimpImporter;
	const aiScene* scene;

	vector<Texture> texturesLoaded;
};

#endif