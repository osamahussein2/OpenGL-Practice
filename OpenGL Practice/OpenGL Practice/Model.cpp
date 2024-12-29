#include "Model.h"

Model::Model(const char* filePath_)
{
	LoadModel(filePath_);
}

Model::~Model()
{
	fileDirectory = "";
	scene = nullptr;
}

void Model::DrawModel(ShaderProgram *shaderProgram_)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i].DrawMesh(shaderProgram_);
	}
}

void Model::LoadModel(string filePath_)
{
	/* aiProcess_Triangulate makes the models transform its primitive shapes into triangles first but only if the model
	consist not entirely of triangles. */

	// aiProcess_FlipUVs filps the texture coordinates on the y-axis where necessary during processing

	// aiProcess_GenNormals creates normal vectors for each vertex if the model doesn't contain normal vectors

	/* aiProcess_SplitLargeMeshes splits larger meshes into smaller sub-meshes which is useful if the renderer has a 
	maximum number of vertices allowed and can only process smaller meshes. */

	/* aiProcess_OptimizeMeshes does the opposite of aiProcess_SplitLargeMeshes, meshes join together into one large
	mesh, reducing drawing calls for optimization. */

	// Abstract the details of loading models in all different formats
	// The second argument of ReadFile() expects several post-processing options
	scene = assimpImporter.ReadFile(filePath_, aiProcess_Triangulate | aiProcess_FlipUVs);
	
	// Check if the scene and the root node of the scene is null, and if the flags returned is incomplete
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		// Return the error to the console and return
		cout << "ERROR::ASSIMP::" << assimpImporter.GetErrorString() << endl;
		return;
	}

	// Retrieve the directory path of the given file path
	fileDirectory = filePath_.substr(0, filePath_.find_last_of('/'));

	/* A recursive function is a function that does some processing and recursively calls the same function with 
	different parameters until a certain condition is met.  */

	// If load model works, process all the scene's nodes (pass in the root node and the scene)
	ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node_, const aiScene* scene_)
{
	// Process all the node's meshes
	for (unsigned int i = 0; i < node_->mNumMeshes; i++)
	{
		// Check each node's mesh indices and retrieve the corresponding mesh by indexing the scene's mMeshes array
		aiMesh* mesh = scene_->mMeshes[node_->mMeshes[i]];

		/* The returned mesh is then passed to the ProcessMesh function that returns a Mesh object that will be stored 
		in the meshes vector */
		meshes.push_back(ProcessMesh(mesh, scene_));
	}

	// Process all the node's children
	for (unsigned int i = 0; i < node_->mNumChildren; i++)
	{
		/* Once all the meshes have been processed, iterate through the node's children and use the ProcessNode function
		to iterate through each its children */
		ProcessNode(node_->mChildren[i], scene_);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh_, const aiScene* scene_)
{
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	// Process the mesh's vertex positions, normals and texture coordinates
	for (unsigned int i = 0; i < mesh_->mNumVertices; i++)
	{
		Vertex vertex;
		vec3 vector;

		vector.x = mesh_->mVertices[i].x;
		vector.y = mesh_->mVertices[i].y;
		vector.z = mesh_->mVertices[i].z;
		vertex.meshPosition = vector;

		if (mesh_->HasNormals())
		{
			vector.x = mesh_->mNormals[i].x;
			vector.y = mesh_->mNormals[i].y;
			vector.z = mesh_->mNormals[i].z;
			vertex.meshNormal = vector;
		}

		// Check if the mesh has texture coordinates
		if (mesh_->mTextureCoords[0])
		{
			vec2 textureVector;

			textureVector.x = mesh_->mTextureCoords[0][i].x;
			textureVector.y = mesh_->mTextureCoords[0][i].y;
			vertex.meshTextureCoordinates = textureVector;
		}

		// But if the mesh doesn't have texture coordinates, set the texture coordinates to 0 on both axes
		else if (!mesh_->mTextureCoords[0])
		{
			vertex.meshTextureCoordinates = vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	// Process the mesh's indices
	for (unsigned int i = 0; i < mesh_->mNumFaces; i++)
	{
		// Iterate through all the mesh faces
		aiFace face = mesh_->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			// Store all the face's indices in the indices vector
			indices.push_back(face.mIndices[j]);
		}
	}

	// Process the mesh's material
	if (mesh_->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene_->mMaterials[mesh_->mMaterialIndex];

		vector<Texture> diffuseMaps = LoadMaterialTexture(material, aiTextureType_DIFFUSE, "TextureDiffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		vector<Texture> specularMaps = LoadMaterialTexture(material, aiTextureType_SPECULAR, "TextureSpecular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

vector<Texture> Model::LoadMaterialTexture(aiMaterial* material_, aiTextureType textureType, string typeName_)
{
	vector<Texture> textures;

	for (unsigned int i = 0; i < material_->GetTextureCount(textureType); i++)
	{
		aiString str;
		material_->GetTexture(textureType, i, &str);

		bool skip = false;

		for (unsigned int j = 0; j < texturesLoaded.size(); j++)
		{
			if (strcmp(texturesLoaded[j].texturePath.data(), str.C_Str()) == 0)
			{
				textures.push_back(texturesLoaded[j]);
				skip = true;

				break;
			}
		}

		if (!skip)
		{
			Texture texture;
			texture.textureID = TextureFromFile(str.C_Str(), fileDirectory);
			texture.textureType = typeName_;
			texture.texturePath = str.C_Str();
			textures.push_back(texture);
			texturesLoaded.push_back(texture);
		}
	}

	return textures;
}

unsigned int Model::TextureFromFile(const char* filePath_, const string &fileDirectory_)
{
	string fileName = string(filePath_);
	fileName = fileDirectory_ + '/' + fileName;

	stbi_set_flip_vertically_on_load(true);

	glGenTextures(1, &textureID);

	data = stbi_load(fileName.c_str(), &width, &height, &nrComponents, 0);

	if (data != nullptr)
	{
		GLenum formatChannel;

		if (nrComponents == 1)
		{
			formatChannel = GL_RED;
		}

		else if (nrComponents == 3)
		{
			formatChannel = GL_RGB;
		}

		else if (nrComponents == 4)
		{
			formatChannel = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, formatChannel, width, height, 0, formatChannel, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}

	else
	{
		cout << "Texture has failed to load at path: " << filePath_ << endl;
		stbi_image_free(data);
	}

	return textureID;
}