#ifndef __MESH_H__
#define __MESH_H__

#include <vector>
#include <unordered_map>

#include "glm/glm.hpp"

#include "Shader.h"
#include "Texture.h"
#include "TextureMapper.h"

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
	// Position
	glm::vec3 Position;
	// Normal
	glm::vec3 Normal;
	// TexCoords
	glm::vec2 TexCoords;
	// Tangent
	glm::vec3 Tangent;
	// Bitangent
	glm::vec3 Bitangent;
	// Bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	// Weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

class Mesh
{
public:
	Mesh() = default;
	
	// Cleanup resource
	void Delete();

	// Mesh Data
	std::vector<Vertex>	vertices;
	std::vector<unsigned int> indices;
	std::unordered_map<TextureType, Texture> textureMap;
	unsigned int VAO;

	// Constructors
	Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::unordered_map<TextureType, Texture>&& textures);
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::unordered_map<TextureType, Texture>& textures);

	void AddTextureIfEmpty(TextureType tType, const std::string& filePath);

	// Render the mesh
	void Draw(const Shader& shader, bool skipTexture);

private:
	// Render data 
	unsigned int VBO;
	unsigned int EBO;

	// Initializes all the buffer objects/arrays
	void SetupMesh();
};

#endif
