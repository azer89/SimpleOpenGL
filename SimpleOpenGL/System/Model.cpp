#include "Model.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "assimp/postprocess.h"

#include "TextureMapper.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

inline glm::mat4 mat4_cast(const aiMatrix4x4& m) 
{ 
	return glm::transpose(glm::make_mat4(&m.a1)); 
}

// Constructor, expects a filepath to a 3D model.
Model::Model(const std::string& path)
{
	LoadModel(path);
}

Model::~Model()
{
	for (Mesh& mesh : meshes)
	{
		mesh.Delete();
	}
}

void Model::AddTextureIfEmpty(TextureType tType, const std::string& filePath)
{
	for (Mesh& mesh : meshes)
	{
		mesh.AddTextureIfEmpty(tType, filePath);
	}
}

// Draws the model, and thus all its meshes
void Model::Draw(const Shader& shader, bool skipTexture)
{
	for (Mesh& mesh : meshes)
	{
		mesh.Draw(shader, skipTexture);
	}
}

// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model::LoadModel(std::string const& path)
{
	// Read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	// Check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cerr << "Error ASSIMP:: " << importer.GetErrorString() << '\n';
		return;
	}

	// Retrieve the directory path of the filepath
	directory = path.substr(0, path.find_last_of('/'));

	// Process ASSIMP's root node recursively
	ProcessNode(scene->mRootNode, scene, glm::mat4(1.0));
}

// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::ProcessNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform)
{
	glm::mat4 nodeTransform = mat4_cast(node->mTransformation);
	glm::mat4 totalTransform = parentTransform * nodeTransform;

	// Process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		// The node object only contains indices to index the actual objects in the scene. 
		// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		
		meshes.push_back(ProcessMesh(mesh, scene, totalTransform));
	}
	// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene, totalTransform);
	}
}

Mesh Model::ProcessMesh(const aiMesh* mesh, const aiScene* scene, const glm::mat4& transform)
{
	// Data to fill
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::unordered_map<TextureType, Texture> textures;

	// Walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;
		glm::vec4 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
		// Positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vector.w = 1;
		vertex.Position = transform * vector;
		// Normals
		if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vector.w = 0;
			vertex.Normal = transform * vector;
		}
		// Texture coordinates
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
			// Tangent
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;
			// Bitangent
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
		}
		else
		{
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}
	// Now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		// Retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}
	// Process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	for (auto& aiTType : TextureMapper::aiTTypeSearchOrder)
	{
		auto count = material->GetTextureCount(aiTType);
		for (unsigned int i = 0; i < count; ++i)
		{
			aiString str;
			material->GetTexture(aiTType, i, &str);
			std::string key = str.C_Str();
			TextureType tType = TextureMapper::GetTextureType(aiTType);

			if (!textureMap.contains(key)) // Make sure never loaded before
			{
				Texture texture(tType, str.C_Str());
				texture.CreateFromImageFile(this->directory + '/' + str.C_Str());
				textureMap[key] = texture;
			}

			if (!textures.contains(tType)) // Only support one image per texture type
			{
				textures[tType] = textureMap[key];
			}
		}
	}

	return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}